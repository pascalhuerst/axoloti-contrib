    void ProcessMidi(Push& p, uint8_t b0, uint8_t data1, uint8_t data2) PFX;
    void PushBipolarBar(Push& p, uint8_t r, uint8_t c, uint8_t v) PFX;
    void PushBrowse(Push& p,uint8_t status, uint8_t data1,uint8_t data2) PFX;
    void PushClearLed(Push& p) PFX;
    void PushClearLowerPads(Push& p) PFX;
    void PushClearPads(Push& p)PFX;
    void PushClearRow(Push& p, uint8_t r) PFX;
    void PushClearUpperPads(Push& p) PFX;
    void PushControlRateHandler(Push& p) PFX;
    void PushCopyRow(Push& p,int8_t r,int to, int from) PFX;
    int8_t PushDeterminePadScaleColour(Push& p, int8_t r, int8_t c) PFX;
    void PushDevice(Push& p,midi_device_t d,int port) PFX;
    void PushDisplayBars(Push& p, KeyValuePair_s* parent) PFX;
    void PushDisplayParams(Push& p, KeyValuePair_s* parent) PFX;
    void PushDisplayScales(Push& p,uint8_t status, uint8_t data1,uint8_t data2) PFX;
    void PushDisplaySession(Push& p, KeyValuePair_s* parent) PFX;
    int PushFormatParamValue(char* buf, KeyValuePair_s* kvp) PFX;
    KeyValuePair_s* PushGetParamObject(KeyValuePair_s* obj,uint8_t idx) PFX;
    KeyValuePair_s* PushGetSeqObject(KeyValuePair_s* obj,uint8_t idx) PFX;
    void PushHandleDevice(Push& p, uint8_t status, uint8_t data1,uint8_t data2) PFX;
    void PushHandlePlayMode(Push& p) PFX;
    void PushHandlePlayNote(Push&p, uint8_t status,uint8_t data1, uint8_t data2) PFX;
    void PushHandleSequencerMode(Push& p) PFX;
    void PushHandleSessionMode(Push& p, uint8_t status, uint8_t data1,uint8_t data2)PFX;
    void PushInit(Push& p) PFX;
    void PushInitDisplayParams(Push& p) PFX;
    void PushInitHandler(Push& p) PFX;
    void PushInitLed(Push& p) PFX;
    void PushInitPads(Push& p) PFX;
    void PushInitSession(Push& p) PFX;
    void PushLockLed(Push& p, bool lock) PFX;
    void PushLockLedRow(Push& p, uint8_t r,bool lock) PFX;
    void PushMidiInMsgHandler(Push& p,midi_device_t dev, uint8_t port, uint8_t b0, uint8_t b1, uint8_t b2) PFX;
    void PushNoteOutput(Push& p,midi_device_t d,int port) PFX;
    int PushNumNotesInScale(int16_t scale) PFX;
    uint8_t PushNumParamObjects(KeyValuePair_s* obj) PFX;
    uint8_t PushNumSeqObjects(KeyValuePair_s* obj) PFX;
    void PushParamBar(Push& p, uint8_t c, KeyValuePair_s* kvp) PFX;
    void PushParamValue(Push& p, uint8_t c, KeyValuePair_s* kvp) PFX;
    void PushSeqName(char* buf,KeyValuePair_s* kvp) PFX;
    void PushSetBlock(Push& p,uint8_t r, uint8_t b, const char* txt, uint8_t len) PFX;
    void PushSetCell(Push& p,uint8_t r, uint8_t c, const char* txt, uint8_t len) PFX;
    void PushSetPad(Push& p, uint8_t r, uint8_t c ,uint8_t colour) PFX;
    void PushSetRow(Push& p,uint8_t r, const char* txt, uint8_t len) PFX;
    void PushUnipolarBar(Push& p, uint8_t r, uint8_t c, uint8_t v) PFX;
    void PushUpdateLed(Push& p) PFX;
    void PushUpdatePads(Push& p)PFX;
    void PushUpdateParamValue(Push& p, KeyValuePair_s* parent, uint8_t encoder, int8_t vel) PFX;
    void PushClockTrigger(Push& p) PFX;
    void PushStartTrigger(Push& p) PFX;
    void PushStopTrigger(Push& p) PFX;

    void PushEnableStep(Push& p, uint8_t seqNr, uint8_t stepNr);
    void PushDisableStep(Push& p, uint8_t seqNr, uint8_t stepNr);
    bool PushGetStep(Push& p, uint8_t seqNr, uint8_t stepNr);
    bool PushToggleStep(Push& p, uint8_t seqNr, uint8_t stepNr);

