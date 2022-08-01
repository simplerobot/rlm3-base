#include "rlm3-string.h"
#include "math.h"


#define KB 1024
#define MB 1024 * 1024


static const char* SafePointerHandling(const char* string)
{
	size_t addr = (size_t)string;
	if (string == NULL)
		return "<NULL>";
	if ((addr % 4) != 0)
		return "<MISALIGNED>";
#ifdef STM32F427xx
	if (
		(addr < 0x00000000 || addr >= 0x00000000 +    2 * MB) && // Aliased to Flash
		(addr < 0x08000000 || addr >= 0x08000000 +    2 * MB) && // Flash
		(addr < 0x10000000 || addr >= 0x10000000 +   64 * KB) && // CCM RAM
		(addr < 0x1FFF0000 || addr >= 0x1FFF0000 +   30 * KB) && // System Memory (Flash)
		(addr < 0x20000000 || addr >= 0x20000000 +  192 * KB) && // SRAM
		(addr < 0xD0000000 || addr >= 0xD0000000 +    8 * MB) && // FMC External SDRAM
		true)
		return "<INVALID>";
#endif
	return string;
}

static void WriteChar(RLM3_Format_Fn fn, void* data, char c)
{
	fn(data, c);
}

static void WriteString(RLM3_Format_Fn fn, void* data, const char* string)
{
	for (const char* s = SafePointerHandling(string); *s != 0; s++)
		WriteChar(fn, data, *s);
}

static void WriteHexDigit(RLM3_Format_Fn fn, void* data, bool upper, uint32_t x)
{
	char c = '?';
	if (0 <= x && x <= 9)
		c = '0' + x;
	if (10 <= x && x <= 15)
		c = (upper ? 'A' : 'a') + x - 10;
	WriteChar(fn, data, c);
}

static void WriteCharSafe(RLM3_Format_Fn fn, void* data, char c)
{
	if (c == '\\')
	{
		WriteChar(fn, data, '\\');
		WriteChar(fn, data, '\\');
	}
	else if (' ' <= c && c <= '~')
	{
		WriteChar(fn, data, c);
	}
	else if (c == '\r')
	{
		WriteChar(fn, data, '\\');
		WriteChar(fn, data, 'r');
	}
	else if (c == '\n')
	{
		WriteChar(fn, data, '\\');
		WriteChar(fn, data, 'n');
	}
	else
	{
		WriteChar(fn, data, '\\');
		WriteChar(fn, data, 'x');
		WriteHexDigit(fn, data, true, (c >> 4) & 0x0F);
		WriteHexDigit(fn, data, true, (c >> 0) & 0x0F);
	}
}

static void WriteStringSafe(RLM3_Format_Fn fn, void* data, const char* string)
{
	for (const char* s = SafePointerHandling(string); *s != 0; s++)
		WriteCharSafe(fn, data, *s);
}

static void WriteUInt(RLM3_Format_Fn fn, void* data, uint32_t value)
{
	char buffer_data[10];
	size_t buffer_size = 0;
	for (uint32_t x = value; buffer_size == 0 || x > 0; x /= 10)
		buffer_data[buffer_size++] = '0' + (x % 10);
	for (size_t i = 0; i < buffer_size; i++)
		WriteChar(fn, data, buffer_data[buffer_size - i - 1]);
}

static void WriteInt(RLM3_Format_Fn fn, void* data, int32_t value)
{
	if (value < 0)
		WriteChar(fn, data, '-');
	WriteUInt(fn, data, (value < 0) ? -value : value);
}

static void WriteHex(RLM3_Format_Fn fn, void* data, bool upper, uint32_t value)
{
	size_t i = 1;
	while (i < 8 && (value >> (4 * i)) != 0)
		i++;
	for (size_t j = 1; j <= i; j++)
		WriteHexDigit(fn, data, upper, (value >> (4 * (i - j))) & 0x0F);
}

static inline int32_t quick_floor(double value)
{
	return (int32_t)value - ((value < 0) ? 1 : 0);
}

static void WriteFloat(RLM3_Format_Fn fn, void* data, double value)
{
	if (signbit(value))
	{
		value = -value;
		WriteChar(fn, data, '-');
	}
	int type = fpclassify(value);
	if (type == FP_NAN)
		WriteString(fn, data, "nan");
	else if (type == FP_INFINITE)
		WriteString(fn, data, "inf");
	else if (type == FP_ZERO)
		WriteString(fn, data, "0.");
	else
	{
		static const size_t PRECISION = 6;
		int32_t log = quick_floor(log10(value));
		// Switch to exponential notation for large or small values.
		int32_t exponent = 0;
		if (log < -5 || log > 9)
		{
			exponent = log;
			value *= pow(0.1, exponent);
			log = 0;
		}
		// Display all digits before the decimal
		if (log < 0)
			log = 0;
		// Round value.
		value += 0.5 * pow(0.1, PRECISION);
		// Check the first digit after rounding.  Correct if needed.
		uint32_t leading_digit = (uint32_t)(value / pow(10.0, log));
		if (leading_digit < 1 && log > 0)
			log--;
		if (leading_digit > 9 && exponent == 0)
			log++;
		if (leading_digit > 9 && exponent != 0)
		{
			exponent++;
			value /= 10;
		}
		// If the leading digit is zero, shift by one more.
		if (log > 0 && (uint32_t)(value / pow(10.0, log)) == 0)
			log--;
		// Print value digit by digit.
		size_t skipped_zeros = 0;
		for (int i = log; i >= -(int)PRECISION; i--)
		{
			double place = pow(10.0, i);
			uint32_t digit = (uint32_t)(value / place);
			// Correct for rounding errors.
			if (digit > 9) digit = 9;
			if (digit < 0) digit = 0;
			// Skip trailing zeros after the decimal point.
			if (i < 0 && digit == 0)
			{
				skipped_zeros++;
				continue;
			}
			// Write any skipped zeros.
			for (size_t j = 0; j < skipped_zeros; j++)
				WriteChar(fn, data, '0');
			skipped_zeros = 0;
			// Write this digit.
			WriteChar(fn, data, '0' + digit);
			// Remove this digit.
			value -= digit * place;
			// Display a decimal point.  We do not hide the decimal point, even if we hide the zeros after it.
			if (i == 0)
				WriteChar(fn, data, '.');
		}
		if (exponent != 0)
		{
			WriteChar(fn, data, 'e');
			WriteInt(fn, data, exponent);
		}
	}
}

extern void RLM3_FnVFormat(RLM3_Format_Fn fn, void* data, const char* format, va_list args)
{
	format = SafePointerHandling(format);
	while (*format != 0)
	{
		char c = *(format++);
		if (c == '%')
		{
			char f = *(format++);
			while ((f >= '0' && f <= '9') || (f == 'z') || (f == 'l') || (f == '.'))
				f = *(format++);
			if (f == 's')
				WriteStringSafe(fn, data, va_arg(args, const char*));
			else if (f == 'd')
				WriteInt(fn, data, va_arg(args, int));
			else if (f == 'f')
				WriteFloat(fn, data, va_arg(args, double));
			else if (f == 'u')
				WriteUInt(fn, data, va_arg(args, unsigned int));
			else if (f == 'x')
				WriteHex(fn, data, false, va_arg(args, unsigned int));
			else if (f == 'X')
				WriteHex(fn, data, true, va_arg(args, unsigned int));
			else if (f == 'c')
				WriteCharSafe(fn, data, va_arg(args, int));
			else if (f == '%')
				WriteChar(fn, data, f);
			else
			{
				WriteString(fn, data, "[unsupported format ");
				WriteCharSafe(fn, data, f);
				WriteChar(fn, data, ']');
				break;
			}
		}
		else
		{
			WriteCharSafe(fn, data, c);
		}
	}
}

extern void RLM3_FnFormat(RLM3_Format_Fn fn, void* data, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	RLM3_FnFormat(fn, data, format, args);
	va_end(args);
}

typedef struct FormatBufferData
{
	char* buffer;
	size_t size;
	size_t cursor;
} FormatBufferData;

static void FormatBufferFunction(void* raw_data, char c)
{
	FormatBufferData* data = (FormatBufferData*)raw_data;
	if (data->cursor < data->size)
		data->buffer[data->cursor] = c;
	data->cursor++;
}

extern size_t RLM3_Format(char* buffer, size_t size, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	size_t result = RLM3_VFormat(buffer, size, format, args);
	va_end(args);
	return result;
}

extern size_t RLM3_VFormat(char* buffer, size_t size, const char* format, va_list args)
{
	size_t cursor = RLM3_VFormatNoNul(buffer, size, format, args);
	if (cursor < size)
		buffer[cursor] = 0;
	else if (size > 0)
		buffer[size - 1] = 0;
	return cursor + 1;
}

extern size_t RLM3_FormatNoNul(char* buffer, size_t size, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	size_t result = RLM3_VFormatNoNul(buffer, size, format, args);
	va_end(args);
	return result;
}

extern size_t RLM3_VFormatNoNul(char* buffer, size_t size, const char* format, va_list args)
{
	FormatBufferData data = { 0 };
	data.buffer = buffer;
	data.size = size;
	data.cursor = 0;

	RLM3_FnVFormat(FormatBufferFunction, &data, format, args);

	return data.cursor;
}

