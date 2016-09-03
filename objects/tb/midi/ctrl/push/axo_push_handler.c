
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

				if (PushGetStep(p, r, c)) {
					PushDisableStep(p, r, c);
					 PushSetPad(p, r, c, PAD_UP_CLR);
				} else {
					PushEnableStep(p, r, c);
					 PushSetPad(p, r, c, PAD_OFF_CLR);

			}



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

// For these: seqNr 0..7 / stepNr 0..7
void PushEnableStep(Push& p, uint8_t seqNr, uint8_t stepNr)
{
	p.sequencer[seqNr] |= (1 << (7-stepNr));
	PushDbgLog("en: %02X", p.sequencer[seqNr]);
}

void PushDisableStep(Push& p, uint8_t seqNr, uint8_t stepNr)
{
	p.sequencer[seqNr] &= ~(1 << (7-stepNr));
	PushDbgLog("dis: %02X", p.sequencer[seqNr]);
}

bool PushGetStep(Push& p, uint8_t seqNr, uint8_t stepNr)
{
	return (p.sequencer[seqNr] & (1 << (7-stepNr)) == true);
}

bool PushToggleStep(Push& p, uint8_t seqNr, uint8_t stepNr)
{
	if (PushGetStep(p, seqNr, stepNr)) {
		PushDisableStep(p, seqNr, stepNr);
		return false;
	}

	PushEnableStep(p, seqNr, stepNr);
	return true;
}




void PushClockTrigger(Push& p)
{
	if (!p.running || p._padMode != Push_SequencerMode) return;

	bool is_16th = (p.clk_24ppq % 6 == 0);
	bool is_4th = (p.clk_24ppq % 24 == 0);

	p.clk_24ppq++;
	p.clk_24ppq %= 96; // Reset each beat

	if (p.clk_24ppq % (96 / p.stepsize) == 0) {
		p.lastStep = p.step;
		p.step++;
		p.step %= 8;

		for (uint8_t i=0; i<8; i++) {
			PushSetPad(p, Seq2Pad(i), Step2Pad(p.step), PAD_NOTE_IN_KEY_CLR);
			PushSetPad(p, Seq2Pad(i), Step2Pad(p.lastStep), PAD_OFF_CLR);
		}
		PushUpdatePads(p);

		//PushUpdateLed(p);
	}
}

void PushStartTrigger(Push& p)
{
	PushDbgLog("Start...");
	p.running = true;
}

void PushStopTrigger(Push& p)
{
	PushDbgLog("Stop...");
	p.clk_24ppq = 0;
	p.clk_1ppq = 0;
	p.step = 0;
	p.running = false;
	PushClearPads(p);
	PushUpdateLed(p);
	PushUpdatePads(p);
}

















