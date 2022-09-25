#include "sndPlayer.hpp"

static const uint32_t sc_prestore_frames = 10;

AudioClass *theAudio;

File myFile;

WavContainerFormatParser theParser;

const int32_t sc_buffer_size = 6144;
uint8_t s_buffer[sc_buffer_size];

uint32_t s_remain_size = 0;
bool ErrEnd = false;

static const uint32_t sc_store_frames = 10;


static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
    puts("Attention!");

    if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
        ErrEnd = true;
    }
}

SoundPlayer::SoundPlayer(SDClass sd) : _sd(sd)
{
    isPlaying = 0;

    pinMode(PIN_D14,OUTPUT);
}

void SoundPlayer::init(void)
{
    while (!_sd.begin())
    {
    /* wait until SD card is mounted. */
    Serial.println("Insert SD card.");
    }

    // start audio system
    theAudio = AudioClass::getInstance();

    theAudio->begin(audio_attention_cb);

    puts("initialization Audio Library");

    /* Set clock mode to normal */

    // theAudio->setRenderingClockMode((fmt.rate <= 48000) ? AS_CLKMODE_NORMAL : AS_CLKMODE_HIRES);
    theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
    /* Set output device to speaker with first argument.
     * If you want to change the output device to I2S,
     * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
     * Set speaker driver mode to LineOut with second argument.
     * If you want to change the speaker driver mode to other,
     * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
     * as an argument.
     */
    theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

    /*
     * Set main player to decode wav. Initialize parameters are taken from wav header.
     * Search for WAV decoder in "/mnt/sd0/BIN" directory
     */
    err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_WAV, "/mnt/sd0/BIN", 44100, 16, 2);

    /* Verify player initialize */
    if (err != AUDIOLIB_ECODE_OK)
    {
        printf("Player0 initialize error\n");
        exit(1);
    }
    isEnd = false;
    isPlaying = 0;
}

void SoundPlayer::play(char *filePath)
{
    fmt_chunk_t fmt;

    handel_wav_parser_t *handle = (handel_wav_parser_t *)theParser.parseChunk(filePath, &fmt);
    if (handle == NULL)
    {
        printf("Wav parser error.\n");
        exit(1);
    }

    // Get data chunk info from wav format
    uint32_t data_offset = handle->data_offset;
    s_remain_size = handle->data_size;

    theParser.resetParser((handel_wav_parser *)handle);
    theAudio->setRenderingClockMode((fmt.rate <= 48000) ? AS_CLKMODE_NORMAL : AS_CLKMODE_HIRES);
    err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_WAV, "/mnt/sd0/BIN", fmt.rate, fmt.bit, fmt.channel);

    /* Verify player initialize */
    if (err != AUDIOLIB_ECODE_OK)
    {
        printf("Player0 initialize error\n");
        exit(1);
    }

    /* Open file placed on SD card */
    printf("%s\n",&filePath[9]);
    myFile = _sd.open(&filePath[9]);

    /* Verify file open */
    if (!myFile)
    {
        printf("File open error\n");
        exit(1);
    }
    printf("Open! %s\n", myFile.name());

    /* Set file position to beginning of data */
    myFile.seek(data_offset);
    totalSize= 0;
    for (uint32_t i = 0; i < sc_prestore_frames; i++)
    {
        size_t supply_size = myFile.read(s_buffer, sizeof(s_buffer));
        totalSize += supply_size;
        s_remain_size -= supply_size;

        err = theAudio->writeFrames(AudioClass::Player0, s_buffer, supply_size);
        if (err != AUDIOLIB_ECODE_OK)
        {
            break;
        }

        if (s_remain_size == 0)
        {
            break;
        }
    }
    
    puts("Play!");
    if(isPlaying == 0){
        isEnd = false;
        theAudio->startPlayer(AudioClass::Player0);
        isPlaying = 1;
    }
    playTime = 0;
}

void SoundPlayer::pause(void){
    if(isPlaying == 2){
        theAudio->startPlayer(AudioClass::Player0);
        isPlaying = 1;
    } else if(isPlaying == 1){
        theAudio->stopPlayer(AudioClass::Player0);
        isPlaying = 2;
    }
}


void SoundPlayer::stop(void)
{
    theAudio->stopPlayer(AudioClass::Player0);
    myFile.close();
    totalSize =  0;
    isPlaying = 0;
    isEnd = true;
}

void SoundPlayer::setVolume(int vol)
{
    theAudio->setVolume(vol);
}

void SoundPlayer::process(void)
{
    if(isPlaying == 0)return;
    static bool is_carry_over = false;
    static size_t supply_size = 0;
    /* Send new frames to decode in a loop until file ends */
    for (uint32_t i = 0; i < sc_store_frames; i++)
    {
        if (!is_carry_over)
        {
            supply_size = myFile.read(s_buffer, (s_remain_size < sizeof(s_buffer)) ? s_remain_size : sizeof(s_buffer));
            totalSize += supply_size;
            playTime = (uint32_t)totalSize / (44100 * 4);
            s_remain_size -= supply_size;
        }
        is_carry_over = false;

        int err = theAudio->writeFrames(AudioClass::Player0, s_buffer, supply_size);

        if (err == AUDIOLIB_ECODE_SIMPLEFIFO_ERROR)
        {
            is_carry_over = true;
            break;
        }

        if (s_remain_size == 0)
        {
            goto stop_player;
        }
    }

    if (ErrEnd)
    {
        printf("Error End\n");
        goto stop_player;
    }

    /* This sleep is adjusted by the time to read the audio stream file.
     * Please adjust in according with the processing contents
     * being processed at the same time by Application.
     *
     * The usleep() function suspends execution of the calling thread for usec
     * microseconds. But the timer resolution depends on the OS system tick time
     * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
     * it will sleep for a longer time than the time requested here.
     */
    /* Don't go further and continue play */

    return;

stop_player:
    isPlaying = 0;
    isEnd = true;
    theAudio->stopPlayer(AudioClass::Player0);
    myFile.close();
}