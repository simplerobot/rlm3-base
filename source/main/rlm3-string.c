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

static size_t WriteChar(char* buffer, size_t size, size_t cursor, char c)
{
	if (cursor < size)
		buffer[cursor] = c;
	return cursor + 1;
}

static size_t WriteString(char* buffer, size_t size, size_t cursor, const char* string)
{
	for (const char* s = SafePointerHandling(string); *s != 0; s++)
		cursor = WriteChar(buffer, size, cursor, *s);
	return cursor;
}

static size_t WriteHexDigit(char* buffer, size_t size, size_t cursor, bool upper, uint32_t x)
{
	char c = '?';
	if (0 <= x && x <= 9)
		c = '0' + x;
	if (10 <= x && x <= 15)
		c = (upper ? 'A' : 'a') + x - 10;
	return WriteChar(buffer, size, cursor, c);
}

static size_t WriteCharSafe(char* buffer, size_t size, size_t cursor, char c)
{
	if (c == '\\')
	{
		cursor = WriteChar(buffer, size, cursor, '\\');
		cursor = WriteChar(buffer, size, cursor, '\\');
	}
	else if (' ' <= c && c <= '~')
	{
		cursor = WriteChar(buffer, size, cursor, c);
	}
	else if (c == '\r')
	{
		cursor = WriteChar(buffer, size, cursor, '\\');
		cursor = WriteChar(buffer, size, cursor, 'r');
	}
	else if (c == '\n')
	{
		cursor = WriteChar(buffer, size, cursor, '\\');
		cursor = WriteChar(buffer, size, cursor, 'n');
	}
	else
	{
		cursor = WriteChar(buffer, size, cursor, '\\');
		cursor = WriteChar(buffer, size, cursor, 'x');
		cursor = WriteHexDigit(buffer, size, cursor, true, (c >> 4) & 0x0F);
		cursor = WriteHexDigit(buffer, size, cursor, true, (c >> 0) & 0x0F);
	}
	return cursor;
}

static size_t WriteStringSafe(char* buffer, size_t size, size_t cursor, const char* string)
{
	for (const char* s = SafePointerHandling(string); *s != 0; s++)
		cursor = WriteCharSafe(buffer, size, cursor, *s);
	return cursor;
}

static size_t WriteUInt(char* buffer, size_t size, size_t cursor, uint32_t value)
{
	char buffer_data[10];
	size_t buffer_size = 0;
	for (uint32_t x = value; buffer_size == 0 || x > 0; x /= 10)
		buffer_data[buffer_size++] = '0' + (x % 10);
	for (size_t i = 0; i < buffer_size; i++)
		cursor = WriteChar(buffer, size, cursor, buffer_data[buffer_size - i - 1]);
	return cursor;
}

static size_t WriteInt(char* buffer, size_t size, size_t cursor, int32_t value)
{
	if (value < 0)
		cursor = WriteChar(buffer, size, cursor, '-');
	return WriteUInt(buffer, size, cursor, (value < 0) ? -value : value);
}

static size_t WriteHex(char* buffer, size_t size, size_t cursor, bool upper, uint32_t value)
{
	size_t i = 1;
	while (i < 8 && (value >> (4 * i)) != 0)
		i++;
	for (size_t j = 1; j <= i; j++)
		cursor = WriteHexDigit(buffer, size, cursor, upper, (value >> (4 * (i - j))) & 0x0F);
	return cursor;
}

static inline int32_t quick_floor(double value)
{
	return (int32_t)value - ((value < 0) ? 1 : 0);
}

static size_t WriteFloat(char* buffer, size_t size, size_t cursor, double value)
{
	if (signbit(value))
	{
		value = -value;
		cursor = WriteChar(buffer, size, cursor, '-');
	}
	int type = fpclassify(value);
	if (type == FP_NAN)
		cursor = WriteString(buffer, size, cursor, "nan");
	else if (type == FP_INFINITE)
		cursor = WriteString(buffer, size, cursor, "inf");
	else if (type == FP_ZERO)
		cursor = WriteString(buffer, size, cursor, "0.");
	else
	{
		static const size_t PRECISION = 6;
		int32_t log = quick_floor(log10(value));
		// Switch to exponential notation for large or small values.
		int32_t exponent = 0;
		if (log < -5 || log > 10)
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
				cursor = WriteChar(buffer, size, cursor, '0');
			skipped_zeros = 0;
			// Write this digit.
			cursor = WriteChar(buffer, size, cursor, '0' + digit);
			// Remove this digit.
			value -= digit * place;
			// Display a decimal point.  We do not hide the decimal point, even if we hide the zeros after it.
			if (i == 0)
				cursor = WriteChar(buffer, size, cursor, '.');
		}
		if (exponent != 0)
		{
			cursor = WriteChar(buffer, size, cursor, 'e');
			cursor = WriteInt(buffer, size, cursor, exponent);
		}
	}
	return cursor;
}

extern size_t RLM3_Format(char* buffer, size_t size, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	size_t cursor = RLM3_VFormat(buffer, size, format, args);
	va_end(args);
	return cursor;
}

extern size_t RLM3_FormatNoNul(char* buffer, size_t size, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	size_t cursor = RLM3_VFormatNoNul(buffer, size, format, args);
	va_end(args);
	return cursor;
}

extern size_t RLM3_VFormat(char* buffer, size_t size, const char* format, va_list args)
{
	size_t cursor = RLM3_VFormatNoNul(buffer, size, format, args);

	cursor = WriteChar(buffer, size, cursor, 0);
	if (cursor > size && size > 0)
		buffer[size - 1] = 0;

	return cursor;
}

extern size_t RLM3_VFormatNoNul(char* buffer, size_t size, const char* format, va_list args)
{
	size_t cursor = 0;
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
				cursor = WriteStringSafe(buffer, size, cursor, va_arg(args, const char*));
			else if (f == 'd')
				cursor = WriteInt(buffer, size, cursor, va_arg(args, int));
			else if (f == 'f')
				cursor = WriteFloat(buffer, size, cursor, va_arg(args, double));
			else if (f == 'u')
				cursor = WriteUInt(buffer, size, cursor, va_arg(args, unsigned int));
			else if (f == 'x')
				cursor = WriteHex(buffer, size, cursor, false, va_arg(args, unsigned int));
			else if (f == 'X')
				cursor = WriteHex(buffer, size, cursor, true, va_arg(args, unsigned int));
			else if (f == 'c')
				cursor = WriteCharSafe(buffer, size, cursor, va_arg(args, int));
			else if (f == '%')
				cursor = WriteChar(buffer, size, cursor, f);
			else
			{
				cursor = WriteString(buffer, size, cursor, "[unsupported format ");
				cursor = WriteCharSafe(buffer, size, cursor, f);
				cursor = WriteChar(buffer, size, cursor, ']');
				break;
			}
		}
		else
		{
			cursor = WriteCharSafe(buffer, size, cursor, c);
		}
	}
	return cursor;
}
