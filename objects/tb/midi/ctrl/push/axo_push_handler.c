#ifndef NOT_QT_CREATOR
#include "axo_push.h"
#include <stdint.h>
#endif

// minimum time between sending, expressed in krate time
// 3000hz , so 3 * ms
#define MIN_TIME (0 * 100)
#define QS 20

struct PushMidiMsgQueue {
	volatile uint8_t read,write;
	uint8_t data[QS][3];
};


static PushMidiMsgQueue pendingMidiMsgs;


enum BtnColour  {
	BtnClr_Off,
	BtnClr_Dim,
	BtnClr_Dim_Blink,
	BtnClr_Dim_Blink_Fast,
	BtnClr_Lit,
	BtnClr_Lit_Blink,
	BtnClr_Lit_Blink_Fast,
};

void PushInitHandler(Push& p) {
	pendingMidiMsgs.read=pendingMidiMsgs.write=0;
	p._shiftHeld=false;
	p._selectHeld=false;
	PushClearUpperPads(p);
	PushClearLowerPads(p);

	/*
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_VOLUME,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_DEVICE,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_BROWSE,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_SHIFT,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_SCALE,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_NOTE,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_SESSION,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_OCT_UP,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_OCT_DOWN,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_IN,BtnClr_Lit);
	MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_OUT,BtnClr_Lit);
*/

}

void PushMidiInMsgHandler(Push& p,midi_device_t dev, uint8_t port, uint8_t b0, uint8_t b1, uint8_t b2) {
	if(dev != p._in_dev || port != p._in_port) return ;

	//   PushDbgLog("PushMidiHandler %i,%i,%i",b0,b1,b2);

	uint8_t data1= b1;
	uint8_t data2= b2;
	uint8_t next  = (pendingMidiMsgs.write + 1 ) % QS;
	if(next==pendingMidiMsgs.read) {
		PushDbgLog("Push Midi Q overflow");
	}
	pendingMidiMsgs.data[next][0]=b0;
	pendingMidiMsgs.data[next][1]=b1;
	pendingMidiMsgs.data[next][2]=b2;
	pendingMidiMsgs.write = next;

	//TODO: This is usually called by control rate handler
	//ProcessMidi(p, b0, b1, b2);
}

void ProcessMidi(Push& p, uint8_t b0, uint8_t data1, uint8_t data2) {

	static uint32_t clk;
	uint8_t status = b0 & 0x00F0;

	if (status == MIDI_NOTE_ON || status == MIDI_NOTE_OFF) {
		switch(data1) {
		default: {
			// check ranges firsts
			//                if (p._padMode==Push_SequencerMode && data1>=NOTE_PAD_START && data1 <=NOTE_PAD_END) {

			if (status == MIDI_NOTE_ON) {
				uint8_t bbb = data1 - NOTE_PAD_START;
				uint8_t r = (bbb & 0xF8) >> 3;
				uint8_t c = bbb & 0x07;
				uint8_t velocity = 50;
				uint8_t step=(7-r)*8 + c;

				if (PushSeqGetStepVelocity(p, p.selectedSeq, step)) {
					PushSeqSetStep(p, p.selectedSeq, step, 0);
				} else {
					PushSeqSetStep(p, p.selectedSeq, step, velocity);
				}
				PushSeqDrawSteps(p, p.selectedSeq);
				//	PushUpdatePads(p);
			}
			//              }
		}
		}
	} else if (status == MIDI_CONTROL_CHANGE) {
		if(data2 == 0) {
			switch(data1) {

			}
		} else {
			/*
			PushSetUpperButton(p, p.selectedSeq, Off);
			p.selectedSeq = data1 - CC_UPPER_PAD_START;
			PushSetUpperButton(p, p.selectedSeq, GreenBright);
			PushSeqDrawSteps(p, data1 - CC_UPPER_PAD_START);
			*/
		}
	}
}

void PushControlRateHandler(Push& p) {
	p._time++;
	while(pendingMidiMsgs.read!=pendingMidiMsgs.write) {
		pendingMidiMsgs.read = (pendingMidiMsgs.read + 1 ) % QS;
		ProcessMidi(p,pendingMidiMsgs.data[pendingMidiMsgs.read][0],pendingMidiMsgs.data[pendingMidiMsgs.read][1],pendingMidiMsgs.data[pendingMidiMsgs.read][2]);
	}

	if(p._time >= MIN_TIME) {
		PushUpdateLed(p);
		PushUpdatePads(p);
		p._time = 0;
	}
}

void PushSeqTickHandler(Push& p)
{
	for (uint8_t i=0; i<8; i++) {
		p.sequencer[i].clk_24ppq_cnt++;
		p.sequencer[i].clk_24ppq_cnt %= 384; //= 16th (?)

		if (p.sequencer[i].clk_24ppq_cnt % p.sequencer[i].ticks_per_step == 0) {
			p.sequencer[i].last_step = p.sequencer[i].step;
			p.sequencer[i].step++;
			p.sequencer[i].step %= p.sequencer[i].step_cnt;
			PushSeqUpdateStep(p, i);
		}
	}
}

void PushSeqUpdateStep(Push& p, uint8_t seqNr)
{
	// Here we decide what kind of view we have. All 8 Seq at once or all 64 steps at once?
	// This is all 8 seq for seq 0:
	uint8_t step = p.sequencer[seqNr].step;
	uint8_t lastStep = p.sequencer[seqNr].last_step;

	if (seqNr == p.selectedSeq) {
		PushSetPad(p, 7-(step/8), step%8, StepActive);
		if (PushSeqGetStepVelocity(p, seqNr, lastStep) > 0) {
			PushSetPad(p, 7-(lastStep/8), lastStep%8, StepSet);
		} else {
			PushSetPad(p, 7-(lastStep/8), lastStep%8, StepUnset);
		}
	}
}

void PushSeqDrawSteps(Push& p, uint8_t seqNr)
{
	for (uint8_t step=0; step<p.sequencer[seqNr].step_cnt; step++) {
		if (p.sequencer[seqNr].velocity[step] > 0) {
			PushSetPad(p, 7-(step/8), step%8, StepSet);
		} else {
			PushSetPad(p, 7-(step/8), step%8, StepUnset);
		}
	}
}

void PushSeqClrStep(Push& p, uint8_t seq, uint8_t step)
{
	p.sequencer[seq].velocity[step] = 0;
}

void PushSeqSetStep(Push& p, uint8_t seq, uint8_t step, uint8_t velocity)
{
	p.sequencer[seq].velocity[step] = velocity;
}

// Returns velocity
uint8_t PushSeqGetStepVelocity(Push& p, uint8_t seq, uint8_t step)
{
	return p.sequencer[seq].velocity[step];
}

void PushClockTrigger(Push& p)
{
	PushSeqTickHandler(p);
}

void PushStartTrigger(Push& p)
{
	PushDbgLog("Start...");
	p.running = true;
	PushSeqInit(p);
}

void PushStopTrigger(Push& p)
{
	PushDbgLog("Stop...");
	p.running = false;
	PushClearPads(p);
	PushSeqDrawSteps(p, p.selectedSeq);
}

void PushSeqInit(Push& p)
{
	for (uint8_t seq=0; seq<8; seq++) {
		p.sequencer[seq].clk_24ppq_cnt = 0;
		p.sequencer[seq].step_cnt = 16+seq;
		p.sequencer[seq].swing = 0;
		p.sequencer[seq].ticks_per_step = seq+2;
		p.sequencer[seq].step = 0;
		p.sequencer[seq].last_step = 0;
		p.sequencer[seq].note = 0x20;
	}
	p.selectedSeq = 0;
}

void PushSetColor(Push& p, uint8_t c) {
	p.color = c;
}













