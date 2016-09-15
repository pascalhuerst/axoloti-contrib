
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

    //MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_SELECT,BtnClr_Lit);
    //MidiSend3(p._out_dev,p._out_port,MIDI_CONTROL_CHANGE,CC_NEW,BtnClr_Lit);

    // initial pad mode
    switch(p._padMode) {
        case Push_PlayMode: {
            PushHandlePlayMode(p);
            break;
        }
        case Push_SequencerMode: {
            PushHandleSequencerMode(p);
            break;
        }
    }
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
                if (p._padMode==Push_PlayMode && data1>=NOTE_PAD_START && data1 <=NOTE_PAD_END) {
                    PushHandlePlayNote(p, status, data1, data2);
                } else if (p._padMode==Push_SequencerMode && data1>=NOTE_PAD_START && data1 <=NOTE_PAD_END) {

			if (status == MIDI_NOTE_ON) {
				uint8_t bbb = data1 - NOTE_PAD_START;
				uint8_t r = (bbb & 0xF8) >> 3;
				uint8_t c = bbb & 0x07;
				uint8_t velocity = 50;


				PushDbgLog("(%i|%i)", c, Pad2Seq(r));

				if (PushSeqGetStepVelocity(p, 7-r, c)) {
				//	PushSeqClrStep(p, 7-r, c);
					PushSeqSetStep(p, 7-r, c, 0);
				} else {
					PushSeqSetStep(p, 7-r, c, velocity);
				}
				PushSeqDrawSteps(p);
				PushUpdatePads(p);
			}
                } else if (p._mode==Push_DeviceMode && data1>=NOTE_ENCODER_START && data1<=NOTE_ENCODER_END) {
                    PushHandleDevice(p, status, data1, data2);
                }
            }
        }
    } else if (status == MIDI_CONTROL_CHANGE) {
        if(data2 == 0) {
            switch(data1) {
                case CC_SHIFT: p._shiftHeld=false; break;
                case CC_SELECT: p._selectHeld=false;break;
            }
        } else {
            switch(data1) {
                // led modes
                case CC_SCALE: {
                    p._mode = Push_ScaleMode;
                    PushDisplayScales(p,status,data1,data2);
                    break;
                }
                case CC_BROWSE: {
                    p._mode = Push_BrowseMode;
                    PushBrowse(p,status,data1,data2);
                    break;
                }
                case CC_DEVICE: {
                    p._mode = Push_DeviceMode;
                    PushHandleDevice(p, status, data1, data2);
                    break;
                }
                case CC_VOLUME: {
                    p._mode = Push_VolumeMode;
                    PushClearLed(p); // tmp for testing
                    break;
                }
                // pad modes
                case CC_NOTE: {
                    p._padMode = Push_PlayMode;
                    PushHandlePlayMode(p);
                    break;
                }
                case CC_SESSION: {
                    p._mode = Push_SessionMode; //!!
                    PushHandleSessionMode(p,status, data1, data2);
                    p._padMode = Push_SequencerMode;
                    PushHandleSequencerMode(p);
                    break;
                }
                // multi function
                case CC_SHIFT: p._shiftHeld=true; break;
                case CC_SELECT: p._selectHeld=true;break;
                case CC_IN: {
                    if(p._mode == Push_DeviceMode) PushHandleDevice(p,status,data1,data2);
                    else if(p._mode == Push_SessionMode) PushHandleSessionMode(p,status, data1, data2);
                    break;
                }
                case CC_OUT: {
                    if(p._mode == Push_DeviceMode) PushHandleDevice(p,status,data1,data2);
                    else if(p._mode == Push_SessionMode) PushHandleSessionMode(p,status, data1, data2);
                    break;
                }
                case CC_OCT_UP: {
                    if(p._padMode == Push_PlayMode) {
                        p._octave = (p._octave<10? p._octave + 1 : p._octave);
                    }
                    break;
                }
                case CC_OCT_DOWN: {
                    if(p._padMode == Push_PlayMode) {
                        p._octave = (p._octave>0? p._octave - 1 : p._octave);
                    }
                    break;
                }
                default: {
                    if(data1>=CC_UPPER_PAD_START && data1<=CC_UPPER_PAD_END) {
                        if(p._mode==Push_ScaleMode) PushDisplayScales(p,status,data1,data2);
                        else if(p._mode==Push_BrowseMode) PushBrowse(p,status,data1,data2);
                    }
                    else if(data1>=CC_LOWER_PAD_START && data1<=CC_LOWER_PAD_END) {
                        if(p._mode==Push_ScaleMode) PushDisplayScales(p,status,data1,data2);
                        else if(p._mode==Push_BrowseMode) PushBrowse(p,status,data1,data2);
                    }
                    else if(data1>=CC_ENCODER_START && data1<=CC_ENCODER_END) {
                        if(p._mode==Push_ScaleMode)  PushDisplayScales(p,status,data1,data2);
                        else if(p._mode == Push_DeviceMode) PushHandleDevice(p,status,data1,data2);
                    }
                    break;
                }
            }
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
	for (uint8_t i=0; i<16; i++) {
		p.sequencer[i].clk_24ppq_cnt++;
		p.sequencer[i].clk_24ppq_cnt %= 384;

		if (p.sequencer[i].clk_24ppq_cnt % p.sequencer[i].ticks_per_step == 0) {
			p.sequencer[i].last_step = p.sequencer[i].step;
			p.sequencer[i].step++;
			p.sequencer[i].step %= p.sequencer[i].step_cnt;
		}
	}
}

void PushSeqDrawSteps(Push& p)
{
	for (uint8_t step=0; step<8; step++) {
		for (uint8_t seq=0; seq<8; seq++) {
			if (PushSeqGetStepVelocity(p, seq, step)) {
				PushSetPad(p, 7-seq, step, 15);
			}
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
	if (!p.running || p._padMode != Push_SequencerMode) return;

	PushSeqTickHandler(p);
	PushUpdatePads(p);
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
	PushUpdatePads(p);
}

void PushSeqInit(Push& p)
{
	for (uint8_t seq=0; seq<8; seq++) {
		p.sequencer[seq].clk_24ppq_cnt = 0;
		p.sequencer[seq].step_cnt = 8;
		p.sequencer[seq].swing = 0;
		p.sequencer[seq].ticks_per_step = 6;
		p.sequencer[seq].step = 0;
		p.sequencer[seq].last_step = 0;
		p.sequencer[seq].note = 0x20;
		for (uint8_t step=0; step<64; step++) {
			p.sequencer[seq].velocity[step] = 0;
		}
	}
}

void PushSetColor(Push& p, uint8_t c) {
	p.color = c;
}













