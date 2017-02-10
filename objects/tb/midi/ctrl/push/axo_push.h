#ifndef AXO_PUSH_H
#define AXO_PUSH_H

#define PUSH_DEBUG_LEVEL 3

extern void LogTextMessage(const char* format, ...);

#if (PUSH_DEBUG_LEVEL > 2)
#define  PushDbgLog(...)   LogTextMessage(__VA_ARGS__)
#else
#define PushDbgLog(...)
#endif


#define PFX __attribute__((noinline))


#define LED_ROWS 4
#define LED_BLOCKS 4
#define LED_CELLS 2 * LED_BLOCKS

// Right of Pads
#define BTN_1_4 	0x24
#define BTN_1_4t 	0x25
#define BTN_1_8		0x26
#define BTN_1_8t	0x27
#define BTN_1_16	0x28
#define BTN_1_16t	0x29
#define BTN_1_32	0x2A
#define BTN_1_32t	0x2B
#define IS_GRID(x)	(x >= BTN_1_4 && x <= BTN_1_32t)

// Stop and Master
#define BTN_STOP	0x1D
#define BTN_MASTER	0x1C


// Right Function Lower Buttons
#define BTN_VOLUME	0x72
#define BTN_TRACK	0x70
#define BTN_DEVICE	0x6E
#define BTN_PAN_SEND	0x73
#define BTN_CLIP	0x71
#define BTN_BROWSE	0x6F

// Right Function Middle Buttons
#define BTN_ENTER		0x3E
#define BTN_ENTER_INV		0x3F
#define BTN_MUTE		0x3C
#define BTN_SOLO		0x3D
#define BTN_SCALES		0x3A
#define BTN_USER		0x3B
#define BTN_REPEAT		0x38
#define BTN_ACCENT		0x39
#define BTN_OCTAVE_DOWN		0x36
#define BTN_OCTAVE_UP		0x37

// Right Function Lower Buttons
#define BTN_ADD_EFFECT		0x34
#define BTN_ADD_TRACK		0x35
#define BTN_NOTE		0x32
#define BTN_SESSION		0x33
#define BTN_SELECT		0x30
#define BTN_SHIFT		0x31


// Right Function Arrows
#define BTN_UP			0x2E
#define BTN_LEFT		0x2C
#define BTN_DOWN		0x2F
#define BTN_RIGHT		0x2D
#define IS_ARROW(x)		(x >= BTN_LEFT && x <= BTN_DOWN)

// Left Function Buttons
#define BTN_UNDO		0x77
#define BTN_DELETE		0x76
#define BTN_DOUBLE		0x75
#define BTN_QUANTIZE		0x74
#define BTN_FIXEDLENGTH		0x5A
#define BTN_AUTOMATION		0x59
#define BTN_DUPLICATE		0x58
#define BTN_NEW			0x57
#define BTN_RECORD		0x56
#define BTN_PLAY		0x55
#define BTN_TEMPO_TAB		0x03
#define BTN_METRONOME		0x09

// Left Encoders
#define ENC_SMALL_1		0x0E
#define ENC_SMALL_2		0x0F
#define IS_SMALL_ENC(x)		(x == ENC_SMALL_1 || x == ENC_SMALL_2)

// Top Encoders
#define ENC_1			0x47
#define ENC_2			0x48
#define ENC_3			0x49
#define ENC_4			0x4A
#define ENC_5			0x4B
#define ENC_6			0x4C
#define ENC_7			0x4D
#define ENC_8			0x4E
#define ENC_9			0x4F
#define ENC_FIRST		ENC_1
#define ENC_LAST		ENC_9
#define IS_ENC(x)		(x >= ENC_FIRST && x <= ENC_LAST)

// Upper Function Buttons
#define BTN_UPPER_1		0x14
#define BTN_UPPER_2		0x15
#define BTN_UPPER_3		0x16
#define BTN_UPPER_4		0x17
#define BTN_UPPER_5		0x18
#define BTN_UPPER_6		0x19
#define BTN_UPPER_7		0x1A
#define BTN_UPPER_8		0x1B
#define BTN_UPPER_FIRST		BTN_UPPER_1
#define BTN_UPPER_LAST		BTN_UPPER_8
#define IS_UPPER_BTN(x)		(x >= BTN_UPPER_FIRST && x <= BTN_UPPER_LAST)

// Lower Function Buttons
#define BTN_LOWER_1		0x66
#define BTN_LOWER_2		0x67
#define BTN_LOWER_3		0x68
#define BTN_LOWER_4		0x69
#define BTN_LOWER_5		0x6A
#define BTN_LOWER_6		0x6B
#define BTN_LOWER_7		0x6C
#define BTN_LOWER_8		0x6D
#define BTN_LOWER_FIRST		BTN_LOWER_1
#define BTN_LOWER_LAST		BTN_LOWER_8
#define IS_LOWER_BTN(x)		(x >= BTN_LOWER_FIRST && x <= BTN_LOWER_LAST)

// PADS
#define NOTE_PAD_START 36
#define NOTE_PAD_END  (NOTE_PAD_START + 64)










struct Push;

struct SeqParams {
	uint8_t velocity[64];	// 0: Step off, others velocity
	uint16_t clk_24ppq_cnt;	// 0..383 -> 1 Bar of 24ppq ticks
	uint8_t step_cnt;	// How many steps does this seq have
	uint8_t swing;		// Swing amount, delays somehow the playback
	uint8_t ticks_per_step; // Count of sticks equals 1 step inc
	uint8_t step;		// 0..63
	uint8_t last_step;	// Step bevor the current one
	uint8_t note;		// Note value thats sent on trigger
};

enum ButtonColor {
	Off = 0,
	RedDark = 1,
	RedDarkSlow,
	RedDarkFast,
	RedBright,
	RedBrightSlow,
	RedBrightFast,
	OrangeDark,
	OrangeDarkSlow,
	OrangeDarkFast,
	OrangeBright,
	OrangeBrightSlow,
	OrangeBrightFast,
	YellowDark,
	YellowDarkSlow,
	YellowDarkFast,
	YellowBright,
	YellowBrightSlow,
	YellowBrightFast,
	GreenBright,
	GreenBrightSlow,
	GreenBrightFast
};

enum StepColor {
	StepActive = 15,
	StepSet = 10,
	StepUnset = 2
};

struct Push {

    // play mode
    uint8_t  _octave;    // current octave
    uint8_t _scaleIdx;
    uint16_t _scale;     // current scale
    uint8_t  _numNotesInScale;   // number of notes in current scale
    uint8_t  _tonic;     // current tonic
    uint8_t  _rowOffset; // current tonic
    bool     _chromatic; // are we in chromatic mode or not


    // display
	unsigned char _led[2][LED_ROWS][9+68];
	uint8_t _activeLed[4];
	bool    _isDirtyLed[LED_ROWS];
	bool    _lockLed[LED_ROWS];

    // pads
	uint8_t _pads[8][8];
	uint8_t _dirtyPads[8]; // 8 binary rows

    // throttle - increases each krate, used for display
    uint32_t _time;

    // midi config
	uint8_t _in_port;
    midi_device_t _in_dev;
	uint8_t _out_port;
    midi_device_t _out_dev;

	uint8_t _note_port;
    midi_device_t _note_dev;

    bool    _shiftHeld;
    bool    _selectHeld;

    // parameter info
    uint8_t _deviceParamPos; // current browser offset
    uint8_t _displayParamBars;

    uint8_t _sessionSeqPos; // current browser offset
    uint8_t _sessionSeqIdx; // current selected seq

    // Seq. V2
    SeqParams sequencer[8];
    uint8_t selectedSeq;


    bool running;
   uint8_t color;
};



static const char* SEQSTR = "KVP_instance";
static uint8_t SEQSTRLEN = strlen(SEQSTR);

extern struct KeyValuePair *ObjectKvpRoot;
extern struct KeyValuePair *ObjectKvps[];



#include <ui.h>

// generated with
// ctags -x --c-kinds=f *.c | awk '{$1=$2=$3=$4="";print $0}' | sed 's/{/PFX;/' > axo_push_funcs.h
#include "./axo_push_funcs.h"


#include "./axo_push.c"
#include "./axo_push_led.c"
#include "./axo_push_handler.c"
#include "./axo_push_params.c"
#include "./axo_push_pads.c"

#endif //AXO_PUSH_H
