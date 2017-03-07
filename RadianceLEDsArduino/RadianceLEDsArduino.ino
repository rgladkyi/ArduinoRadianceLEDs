
#define ARDUINO_NANO_BOARD 1

#if defined ARDUINO_NANO_BOARD || defined ARDUINO_PRO_MINI_BOARD || \
ARDUINO_UNO_BOARD || ARDUINO_MEGA_BOARD
int pinRedLED = 3;
int pinGreenLED = 5;
int pinBlueLED = 6;
#else
#error Need to define Pinouts.
#endif

#define DEBUG 0

#if DEBUG
#define DEBUG_LOG_TO_CONSOLE 0
#endif

#if DEBUG
#define PRINT(v) Serial.print(__LINE__); \
    Serial.print(" __LINE__ | VALUE ");Serial.println((v));

#define PRINTH(v) Serial.print(__LINE__); \
    Serial.print(" __LINE__ | VALUE ");Serial.println((v), HEX);
#else
#define PRINT(v)
#define PRINTH(v)
#endif

#define PAUSE delay(1000);

const int COLOR_MAX_VALUE = 255;
const int COLOR_ALPHA_MAX_VALUE = 255;
const int COLOR_ALPHA_DEFAULT_VALUE = COLOR_ALPHA_MAX_VALUE;

const int COLOR_ALPHA_DEFAULT_FADE_STEP = 5;
const int COLOR_ALPHA_DEFAULT_FADE_DELAY_MS = 5;
//const int COLOR_ALPHA_DEFAULT_FADE_DELAY_MS = 30;

typedef unsigned long ulong;

typedef struct
{
    byte red;
    byte green;
    byte blue;
} RGB;

typedef struct
{
    byte alpha; // Opacity of the color, alpha channel: in current
    // implementation means as bridges.
    byte red;
    byte green;
    byte blue;
} ARGB;

RGB LEDsCurrentColor;

RGB getRGB(byte red, byte green, byte blue)
{
    RGB toRGB = {red, green, blue};

    return toRGB;
}

RGB getRGB(ulong rgb)
{
    RGB toRGB = {(rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff };

    return toRGB;
}

ARGB getARGB(byte alpha, byte red, byte green, byte blue)
{
    ARGB toARGB = {alpha, red, green, blue };

    return toARGB;
}

ARGB getARGB(ulong argb)
{
    ARGB toARGB = {(argb >> 24), (argb >> 16) & 0xff, (argb >> 8) & 0xff,
        argb & 0xff };

    return toARGB;
}

ARGB RGBToARGB(RGB rgb, byte alpha)
{
    ARGB toARGB = {alpha, rgb.red, rgb.green, rgb.blue };

    return toARGB;
}

ARGB RGBToARGB(RGB rgb)
{
    return RGBToARGB(rgb, COLOR_ALPHA_DEFAULT_VALUE);
}

RGB ARGBToRGB(ARGB argb)
{
    float coeficient = argb.alpha / (float) COLOR_ALPHA_MAX_VALUE;
    RGB toRGB = {(byte)(argb.red * coeficient), (byte)(argb.green * coeficient),
            (byte)(argb.blue * coeficient)};

    return toRGB;
}

ulong ARGBToULong(ARGB argb)
{
    ulong toULong = ((ulong)((ulong)argb.alpha << 24) + (ulong)(argb.red << 16)
            + (ulong)(argb.green << 8) + (ulong)argb.blue);

    return toULong;
}

ulong RGBToULong(RGB rgb)
{
    return ARGBToULong(RGBToARGB(rgb));
}

static void lightRedLed(byte value)
{
    analogWrite(pinRedLED, value);
    LEDsCurrentColor.red = value;
}

static void lightGreenLed(byte value)
{
    analogWrite(pinGreenLED, value);
    LEDsCurrentColor.green = value;
}

static void lightBlueLed(byte value)
{
    analogWrite(pinBlueLED, value);
    LEDsCurrentColor.blue = value;
}

static byte getLightRedLed()
{
    return LEDsCurrentColor.red;
}

static byte getLightGreenLed()
{
    return LEDsCurrentColor.green;
}

static byte getLightBlueLed()
{
    return LEDsCurrentColor.blue;
}

void lightRGBColor(RGB rgb)
{
    lightRedLed(rgb.red);
    lightGreenLed(rgb.green);
    lightBlueLed(rgb.blue);
}

void lightARGBColor(ARGB argb)
{
    lightRGBColor(ARGBToRGB(argb));
}

RGB getLightRGBColor()
{
    return LEDsCurrentColor;
}

void fadeInFromARGBColor(ARGB argbFrom, ulong fadingDelayMs)
{
    ARGB fadingARGB = argbFrom;

    for (short fadingAlpha = argbFrom.alpha; fadingAlpha >= 0; fadingAlpha--)
    {
        // Stop when related RGB color is off.
        // Mean's is off without alpha byte.
        if (!RGBToULong(ARGBToRGB(fadingARGB)))
        {
            break;
        }

        fadingARGB.alpha = fadingAlpha;
        lightARGBColor(fadingARGB);
        delay(fadingDelayMs);
    }
}

void fadeInRGBColor(RGB rgb, ulong fadingDelayMs)
{
    fadeInFromARGBColor(RGBToARGB(rgb), fadingDelayMs);
}

void fadeOutToARGBColor(ARGB argbTo, ulong fadingDelayMs)
{
    ARGB fadingOutARGB = argbTo;

    for (short fadingOutAlpha = 0; fadingOutAlpha <= argbTo.alpha;
            fadingOutAlpha++)
    {
        // Miss the conditions when RGB color is off,
        // but alpha is not.
        if (!RGBToULong(ARGBToRGB(fadingOutARGB)))
        {
            continue;
        }

        fadingOutARGB.alpha = fadingOutAlpha;
        lightARGBColor(fadingOutARGB);
        delay(fadingDelayMs);
    }
}

void fadeOutToRGBColor(RGB rgb, ulong fadingDelayMs)
{
    fadeOutToARGBColor(RGBToARGB(rgb), fadingDelayMs);
}

void transitARGBColorFromTo(ARGB argbFrom, ARGB argbTo, ulong fadingDelayMs)
{
    ARGB transitARGB = argbFrom;
    float transitCoefficient;

    // Transition is performed just for next state, means start from '1'.
    for (short transit = 1; transit <= COLOR_MAX_VALUE; transit++)
    {
        // Stop when nothing to transit anymore.
        if ((ARGBToULong(transitARGB) == ARGBToULong(argbTo)))
        {
            break;
        }

        transitCoefficient = transit / (float)COLOR_MAX_VALUE;

        // Difference multiplied to transit coefficient gives the next state.
        transitARGB.alpha = argbFrom.alpha + (short)(((short)argbTo.alpha -
            (short)argbFrom.alpha) * transitCoefficient);
        transitARGB.red = argbFrom.red + (short)(((short)argbTo.red -
            (short)argbFrom.red) * transitCoefficient);
        transitARGB.green = argbFrom.green + (short)(((short)argbTo.green -
            (short)argbFrom.green) * transitCoefficient);
        transitARGB.blue = argbFrom.blue + (short)(((short)argbTo.blue -
            (short)argbFrom.blue) * transitCoefficient);

        lightARGBColor(transitARGB);
        delay(fadingDelayMs);
    }
}

void transitRGBColorFromTo(RGB rgbFrom, RGB rgbTo, ulong fadingDelayMs)
{
    transitARGBColorFromTo(RGBToARGB(rgbFrom), RGBToARGB(rgbTo),
        fadingDelayMs);
}

void testLEDsRGBBasics()
{
    lightRedLed(0);
    lightGreenLed(0);
    lightBlueLed(0);
    PAUSE;
    lightRedLed(1);
    lightGreenLed(1);
    lightBlueLed(1);
    PAUSE;
    lightRedLed(254);
    lightGreenLed(254);
    lightBlueLed(254);
    PAUSE;
    lightRedLed(255);
    lightGreenLed(255);
    lightBlueLed(255);


    lightARGBColor(getARGB(255, 255, 255, 255));
    PAUSE;
    lightARGBColor(getARGB(254, 255, 255, 255));
    PAUSE;
    lightARGBColor(getARGB(2, 255, 255, 255));
    PAUSE;
    lightARGBColor(getARGB(1, 255, 255, 255));
    PAUSE;
    lightARGBColor(getARGB(0, 255, 255, 255));
    PAUSE;
    lightARGBColor(getARGB(255, 255, 255, 255));
    PAUSE;
}

void testRGB(short r, short g, short b)
{
#if DEBUG_LOG_TO_CONSOLE

    Serial.println();
    Serial.print("Test the RGB: 0x");
    Serial.print(RGBToULong(getRGB(r, g, b)), HEX);
    Serial.print(" == 0x");
    Serial.print(RGBToULong(getRGB(RGBToULong(getRGB(r, g, b)))), HEX);
    Serial.print(" == 0x");
    Serial.print(ARGBToULong(RGBToARGB(getRGB(r, g, b),
                    COLOR_ALPHA_DEFAULT_VALUE)), HEX);
    Serial.print(" == 0x");
    Serial.print(ARGBToULong(RGBToARGB(getRGB(r, g, b))), HEX);

    Serial.println();
    Serial.print("Test the ARGB: 0x");
    Serial.print(ARGBToULong(getARGB(COLOR_ALPHA_DEFAULT_VALUE, r, g, b)), HEX);
    Serial.print(" == 0x");
    Serial.print(ARGBToULong(getARGB(ARGBToULong(getARGB(
                            COLOR_ALPHA_DEFAULT_VALUE, r, g, b)))), HEX);
    Serial.print(" == 0x");
    Serial.print(RGBToULong(ARGBToRGB(getARGB(COLOR_ALPHA_DEFAULT_VALUE, r, g,
                        b))), HEX);
#endif

    lightRGBColor(getRGB(r, g, b));

#if DEBUG_LOG_TO_CONSOLE
    Serial.println();    fadeOutToARGBColor(RGBToARGB(getRGB(255, 255, 255)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    Serial.println("Test the LEDs:");

    Serial.print("Current RGB is 0x");
    Serial.print(getLightRedLed(), HEX);
    Serial.print(getLightGreenLed(), HEX);
    Serial.print(getLightBlueLed(), HEX);
    Serial.print(" == 0x");
    Serial.print(RGBToULong(getLightRGBColor()), HEX);
#endif

#if DEBUG_LOG_TO_CONSOLE
    delay(1);
#else
    delay(COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
#endif
}

void testSingleLED()
{
    for (short red = 0; red <= COLOR_MAX_VALUE; red++)
    {
        testRGB(red, 0, 0);
    }

    for (short green = 0; green <= COLOR_MAX_VALUE; green++)
    {
        testRGB(255, green, 0);
    }

    for (short blue = 0; blue <= COLOR_MAX_VALUE; blue++)
    {
        testRGB(255, 255, blue);
    }
}

void testARGBAlphaChannel()
{
    for (short light = COLOR_MAX_VALUE; light >= 0; light--)
    {
        for (short alpha = COLOR_ALPHA_MAX_VALUE; alpha >= 0; alpha--)
        {
            lightARGBColor(getARGB(alpha, light, light, light));

#if !DEBUG_LOG_TO_CONSOLE
            delay(1);
#endif
        }
    }
}

void testARGBFaidingAndTransiting()
{
    lightRGBColor(getRGB(255, 255, 255));
    PAUSE;

    fadeInFromARGBColor(RGBToARGB(getRGB(0, 0, 0)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);

    fadeInFromARGBColor(RGBToARGB(getRGB(1, 1, 1)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);


    fadeInFromARGBColor(RGBToARGB(getRGB(254, 254, 254)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);

    fadeInFromARGBColor(RGBToARGB(getRGB(255, 255, 255)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);


    lightRGBColor(getRGB(255, 255, 255));
    PAUSE;

    fadeOutToARGBColor(RGBToARGB(getRGB(0, 0, 0)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);

    fadeOutToARGBColor(RGBToARGB(getRGB(1, 1, 1)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);


    fadeOutToARGBColor(RGBToARGB(getRGB(254, 254, 254)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);

    fadeOutToARGBColor(RGBToARGB(getRGB(255, 255, 255)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    fadeOutToARGBColor(RGBToARGB(getRGB(255, 255, 255)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    fadeOutToARGBColor(RGBToARGB(getRGB(255, 255, 255)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    fadeOutToARGBColor(RGBToARGB(getRGB(255, 255, 255)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    fadeOutToARGBColor(RGBToARGB(getRGB(255, 255, 255)),
            COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);

    RGB rgb = getRGB(255, 5, 70);

    lightRGBColor(rgb);
    PAUSE;

    for (int n = 0; n < 10; n++)
    {
        fadeInRGBColor(rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
        fadeOutToRGBColor(rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    }

    RGB rgb_prev = getRGB(random(256), random(256), random(256));

    for (int n = 0; n < 30; n++)
    {
        rgb = getRGB(random(256), random(256), random(256));

        fadeOutToRGBColor(rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
        fadeInRGBColor(rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
        rgb_prev = rgb;
    }

    for (int n = 0; n < 30; n++)
    {
        transitRGBColorFromTo(rgb_prev, rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
        rgb_prev = rgb;
    }

}

void setup()
{
    Serial.begin(9600);
}

#define EXECUTE_SINGLE(is_action_was_executting, action_was_executting) \
    static bool is_action_was_executting = false; \
    bool action_was_executting = false; \
    if (!is_action_was_executting) { \
        is_action_was_executting = true; \
        action_was_executting = true; } \
    if (action_was_executting)

void loop()
{
#if DEBUG
    EXECUTE_SINGLE(is_test_cases_was_executting, test_cases_was_executting)
    {
        testLEDsRGBBasics();
        testSingleLED();
        testARGBAlphaChannel();
        testARGBFaidingAndTransiting();

        lightRGBColor(getRGB(0, 0, 0));
    }
#else
    static RGB rgb, rgb_prev = getRGB(random(256), random(256), random(256));

    rgb = getRGB(random(256), random(256), random(256));

/*
    fadeOutToRGBColor(rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    fadeInRGBColor(rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
*/
    transitRGBColorFromTo(rgb_prev, rgb, COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    rgb_prev = rgb;

//lightRGBColor(getRGB(50, 50, 5));

  /* transitRGBColorFromTo(getRGB(255, 40, 80), getRGB(200, 100, 5),
        COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
    transitRGBColorFromTo(getRGB(200, 100, 5), getRGB(255, 40, 80),
        COLOR_ALPHA_DEFAULT_FADE_DELAY_MS);
*/
   /*     transitRGBColorFromTo(getRGB(100, 100, 100), getRGB(0, 0, 0),
        COLOR_ALPHA_DEFAULT_FADE_DELAY_MS * 5);
    transitRGBColorFromTo(getRGB(0, 0, 0), getRGB(100, 100, 100),
        COLOR_ALPHA_DEFAULT_FADE_DELAY_MS * 5);*/
#endif
}

