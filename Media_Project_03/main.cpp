#include "utils.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sapi.h>
#include <sphelper.h>

FMOD_RESULT mresult;
FMOD::System *msystem = NULL;
FMOD::Channel *pChannel[5];
FMOD::Sound *pSound[5];

#define NUMBER_OF_TAGS 4
#define DEVICE_INDEX 0
#define STREAM_BUFFER_SIZE 65536

char* url_smooth_lounge = "http://sl128.hnux.com";
char* url_holiday = "http://sc-christmas.1.fm:8650/";
char* url_tormented_radio = "http://stream2.mpegradio.com:8070";

char* genre[3] = { "?", "?", "?" };
char* artist[3] = { "?", "?", "?" };
char* title[3] = { "?", "?", "?" };

std::string speech1[9];
std::string speech2 = "";

//Create voice object.
ISpVoice *pVoice = NULL; //This voice is for direct output
CComPtr<ISpVoice> cpVoice; //This voice is for recording to a wav file
HRESULT hr;
CSpStreamFormat	cAudioFormat;
CComPtr<ISpStream> cpStream;

FMOD_OPENSTATE mopenstate = FMOD_OPENSTATE_READY;
int mtag_index = 0;
char mtag_string[NUMBER_OF_TAGS][1028] = { 0 };

//DSP variables
FMOD::DSP* dsp1 = 0;
FMOD::DSP* dsp2 = 0;
FMOD::DSP* dsp3 = 0;

FMOD::DSP* dsp4 = 0;
FMOD::DSP* dsp5 = 0;
FMOD::DSP* dsp6 = 0;

int chosenStream = 0;
bool mis_esc = false;
bool mkeydown = false;
int sleep_ms = 50;

void errorcheck(FMOD_RESULT result) {
	if (result != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

void ugly_clear_screen() {
	//Needed for print_text
	start_text();

	for (int i = 0; i < 20; i++)
	{
		print_text("																			");
	}

	//Needed for print_text
	end_text();

}

void handle_keyboard() {

	//===============================================================================================
	//Esc key pressed
	if (GetAsyncKeyState(VK_ESCAPE)) {
		mis_esc = true;
	}

	//===============================================================================================
	// Space bar pressed		
	if ((GetKeyState(VK_SPACE) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE

		ugly_clear_screen();
		chosenStream = 0;
		pChannel[0]->setPaused(true);
		pChannel[1]->setPaused(true);
		pChannel[2]->setPaused(true);
		mresult = msystem->recordStop(DEVICE_INDEX);
		errorcheck(mresult);

		pChannel[3]->setPaused(true);

		if (SUCCEEDED(hr)) 
		{
			//Sentence 1: Female voice
			std::string mysentence = "<voice required='Gender = Female'>" + speech1[0] + "</voice>";
			std::wstring stemp = s2ws(mysentence);
			LPCWSTR result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 2: Female voice, spells out sentence
			mysentence = "<voice required='Gender = Female'><spell>" + speech1[1] + "</spell></voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 3: Female voice, emphasize some words (hard to hear. Easier to hear with male voice)
			mysentence = "<voice required='Gender = Female'>" + speech1[2].substr(0, 34) + "<emph>" + speech1[2].substr(34, 20) + "</emph>" + speech1[2].substr(54, std::string::npos) + "</voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 4: Male voice, rate -5
			mysentence = "<voice required='Gender = Male'><rate absspeed='-5'/>" + speech1[3] + "</voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 5: Male voice, rate 10
			mysentence = "<voice required='Gender = Male'><rate absspeed='10'/>" + speech1[4] + "</voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 6: Male voice, rate 15
			mysentence = "<voice required='Gender = Male'><rate absspeed='15'/>" + speech1[5] + "</voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 7: Female voice, 800ms pause in the middle
			mysentence = "<voice required='Gender = Female'>" + speech1[6].substr(0, 79) + "<silence msec = '800'/>" + speech1[6].substr(79, std::string::npos) + "</voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 8: Female voice, Pitched up to 5
			mysentence = "<voice required='Gender = Female'><pitch absmiddle='5'/>" + speech1[7] + "</voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);

			//Sentence 9: Female voice, Pitched up to 10
			mysentence = "<voice required='Gender = Female'><pitch absmiddle='10'/>" + speech1[8] + "</voice>";
			stemp = s2ws(mysentence);
			result = stemp.c_str();
			hr = pVoice->Speak(result, 0, NULL);
		}

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Enter key
	else if ((GetKeyState(VK_RETURN) < 0) && !mkeydown) {
		bool thePaused;
		pChannel[4]->getPaused(&thePaused);
		pChannel[4]->setPaused(!thePaused);
	}

	//===============================================================================================
	//Number 1
	else if ((GetKeyState(0x31) < 0) && !mkeydown) { //Key down
		mkeydown = true;
		//YOUR CODE HERE
		if (chosenStream == 0)
		{
			bool thePaused;
			pChannel[0]->getPaused(&thePaused);
			pChannel[0]->setPaused(!thePaused);
		}
		else
		{
			ugly_clear_screen();
			chosenStream = 0;
			pChannel[0]->setPaused(false);
			pChannel[1]->setPaused(true);
			pChannel[2]->setPaused(true);
			mresult = msystem->recordStop(DEVICE_INDEX);
			errorcheck(mresult);

			pChannel[3]->setPaused(true);
		}

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 2
	else if ((GetKeyState(0x32) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		if (chosenStream == 1)
		{
			bool thePaused;
			pChannel[1]->getPaused(&thePaused);
			pChannel[1]->setPaused(!thePaused);
		}
		else
		{
			ugly_clear_screen();
			chosenStream = 1;
			pChannel[0]->setPaused(true);
			pChannel[1]->setPaused(false);
			pChannel[2]->setPaused(true);
			mresult = msystem->recordStop(DEVICE_INDEX);
			errorcheck(mresult);

			pChannel[3]->setPaused(true);
		}

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 3
	else if ((GetKeyState(0x33) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		if (chosenStream == 2)
		{
			bool thePaused;
			pChannel[2]->getPaused(&thePaused);
			pChannel[2]->setPaused(!thePaused);
		}
		else
		{
			ugly_clear_screen();
			chosenStream = 2;
			pChannel[1]->setPaused(true);
			pChannel[0]->setPaused(true);
			pChannel[2]->setPaused(false);
			mresult = msystem->recordStop(DEVICE_INDEX);
			errorcheck(mresult);

			pChannel[3]->setPaused(true);
		}

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 4
	else if ((GetKeyState(0x34) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		if (chosenStream == 4)
		{
			bool thePaused;
			pChannel[3]->getPaused(&thePaused);
			if (thePaused)
			{
				pChannel[3]->setPaused(false);
				//Start recording
				mresult = msystem->recordStart(DEVICE_INDEX, pSound[3], true);
				errorcheck(mresult);
			}
			else
			{
				mresult = msystem->recordStop(DEVICE_INDEX);
				errorcheck(mresult);

				pChannel[3]->setPaused(true);
			}
		}
		else
		{
			ugly_clear_screen();
			chosenStream = 4;
			pChannel[1]->setPaused(true);
			pChannel[0]->setPaused(true);
			pChannel[2]->setPaused(true);
			pChannel[3]->setPaused(false);

			//Start recording
			mresult = msystem->recordStart(DEVICE_INDEX, pSound[3], true);
			errorcheck(mresult);
		}

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 5
	else if ((GetKeyState(0x35) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dsp2->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dsp2->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 6
	else if ((GetKeyState(0x36) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dsp3->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dsp3->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 7
	else if ((GetKeyState(0x37) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dsp1->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dsp1->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 8
	else if ((GetKeyState(0x38) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dsp4->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dsp4->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 9
	else if ((GetKeyState(0x39) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dsp5->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dsp5->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 0
	else if ((GetKeyState(0x30) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dsp6->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dsp6->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}
}

int main()
{
	// Create the main system object.
	mresult = FMOD::System_Create(&msystem);
	if (mresult != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", mresult, FMOD_ErrorString(mresult));
		exit(-1);
	}

	//Initializes the system object, and the msound device. This has to be called at the start of the user's program
	mresult = msystem->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (mresult != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", mresult, FMOD_ErrorString(mresult));
		exit(-1);
	}

	//Load our first speech
	std::ifstream speechFile("Speech_1.txt");
	if (!speechFile.is_open())
		return 0;

	std::string curLine;

	for (int i = 0; i < 9; i++)
	{
		bool punctuation = false;

		while(!punctuation)
		{
			speechFile >> curLine;
			speech1[i] += curLine + " ";

			if (curLine[curLine.length() - 1] == ',' || curLine[curLine.length() - 1] == '.')
			{
				punctuation = true;
			}
		}
	}

	//Load our second speech
	std::ifstream speechFile2("Speech_2.txt");
	if (!speechFile.is_open())
		return 0;

	while (!speechFile2.eof()) 
	{
		speechFile2 >> curLine;
		speech2 += curLine + " ";
	}

	//Initialize the COM library on the current thread
	if (FAILED(::CoInitialize(NULL))) 
	{
		return false;
	}

	//Initialize voice.
	hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);

	hr = cpVoice.CoCreateInstance(CLSID_SpVoice);

	//Set audio format	
	if (SUCCEEDED(hr)) 
	{
		hr = cAudioFormat.AssignFormat(SPSF_22kHz16BitStereo);
	}

	//Bind stream to file
	if (SUCCEEDED(hr)) 
	{
		hr = SPBindToFile(L"c:\Medos_Alex\project3.wav", SPFM_CREATE_ALWAYS, &cpStream, &cAudioFormat.FormatId(), cAudioFormat.WaveFormatExPtr());
	}
	
	//Set output to cpstream
	if (SUCCEEDED(hr)) 
	{
		hr = cpVoice->SetOutput(cpStream, true);
	}

	//Record the audio
	if (SUCCEEDED(hr))
	{
		std::wstring stemp = s2ws(speech2);
		LPCWSTR result = stemp.c_str();
		hr = cpVoice->Speak(result, SPF_DEFAULT, NULL);
	}

	//Close the stream
	if (SUCCEEDED(hr)) 
	{
		hr = cpStream->Close();
	}

	cpStream.Release();
	cpVoice.Release();

	//Identify recording drivers in your pc
	int numberofdrivers = 0;

	int numberoftries = 20;
	while (numberoftries > 0 && numberofdrivers <= 0)
	{
		numberoftries--;
		Sleep(100);
		mresult = msystem->getRecordNumDrivers(0, &numberofdrivers);
		errorcheck(mresult);
	}

	if (numberofdrivers == 0)
	{
		//no recording devices available. 
		exit(EXIT_FAILURE);
	}

	//Get recording driver info
	int nativeRate = 0;
	int nativeChannels = 0;
	mresult = msystem->getRecordDriverInfo(DEVICE_INDEX, NULL, 0, NULL, &nativeRate, NULL, &nativeChannels, NULL);
	errorcheck(mresult);

	//Create user sound
	FMOD_CREATESOUNDEXINFO exinfo = { 0 };
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels = nativeChannels;
	exinfo.format = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = nativeRate;
	exinfo.length = nativeRate * sizeof(short) * nativeChannels;

	//Increase internal buffersize for streams opened to account for Internet lag, default is 16384 
	mresult = msystem->setStreamBufferSize(STREAM_BUFFER_SIZE, FMOD_TIMEUNIT_RAWBYTES);
	errorcheck(mresult);

	//Create sound using an intenet url
	mresult = msystem->createSound(url_smooth_lounge, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &pSound[0]);
	errorcheck(mresult);

	mresult = msystem->createSound(url_holiday, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &pSound[1]);
	errorcheck(mresult);

	mresult = msystem->createSound(url_tormented_radio, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &pSound[2]);
	errorcheck(mresult);

	//Create sound from user input
	mresult = msystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &exinfo, &pSound[3]);
	errorcheck(mresult);

	//Play recording sound
	mresult = msystem->playSound(pSound[3], 0, true, &pChannel[3]);
	errorcheck(mresult);

	//Create sound from Text To Speech recorded .wav file
	mresult = msystem->createSound("Medos_Alexproject3.wav", FMOD_CREATESAMPLE, 0, &pSound[4]);
	errorcheck(mresult);

	mresult = msystem->playSound(pSound[4], 0, true, &pChannel[4]);
	errorcheck(mresult);

	pSound[4]->setMode(FMOD_LOOP_NORMAL);

	//Create DSP effects
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_ECHO, &dsp2);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_FLANGE, &dsp3);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &dsp1);
	errorcheck(mresult);

	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_ECHO, &dsp4);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_FLANGE, &dsp5);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &dsp6);
	errorcheck(mresult);

	//Add effects to the voice recording
	mresult = pChannel[3]->addDSP(0, dsp2);
	errorcheck(mresult);
	mresult = pChannel[3]->addDSP(0, dsp3);
	errorcheck(mresult);
	mresult = pChannel[3]->addDSP(0, dsp1);
	errorcheck(mresult);

	mresult = pChannel[4]->addDSP(0, dsp4);
	errorcheck(mresult);
	mresult = pChannel[4]->addDSP(0, dsp5);
	errorcheck(mresult);
	mresult = pChannel[4]->addDSP(0, dsp6);
	errorcheck(mresult);

	//Bypass all effects, this plays the sound with no effects.
	mresult = dsp2->setBypass(true);
	errorcheck(mresult);
	mresult = dsp3->setBypass(true);;
	errorcheck(mresult);
	mresult = dsp1->setBypass(true);
	errorcheck(mresult);

	mresult = dsp4->setBypass(true);
	errorcheck(mresult);
	mresult = dsp5->setBypass(true);;
	errorcheck(mresult);
	mresult = dsp6->setBypass(true);
	errorcheck(mresult);

	unsigned int position = 0;
	unsigned int percentage = 0;
	bool is_playing = false;
	bool is_paused = false;
	bool is_starving = false;
	const char* current_state = "Stopped";
	FMOD_TAG tag;

	while (!mis_esc)
	{
		//Needed for print_text
		start_text();

		handle_keyboard();

		//Important to update msystem
		mresult = msystem->update();
		errorcheck(mresult);

		//Get stream sound state
		if (chosenStream < 4)
		{
			mresult = pSound[chosenStream]->getOpenState(&mopenstate, &percentage, &is_starving, 0);
			errorcheck(mresult);

			if (pChannel[chosenStream])
			{

				//TODO:
				//Retrieve descriptive tag stored by the sound
				while (pSound[chosenStream]->getTag(0, -1, &tag) == FMOD_OK)
				{
					std::string tagName = tag.name;
					if (tag.datatype == FMOD_TAGDATATYPE_STRING)
					{
						if (tagName == "ARTIST")
						{
							artist[chosenStream] = (char*)tag.data;
						}
						else if (tagName == "TITLE")
						{
							title[chosenStream] = (char*)tag.data;
						}
						else if (tagName == "icy-genre")
						{
							genre[chosenStream] = (char*)tag.data;
						}
						//sprintf_s(mtag_string[mtag_index], "%s = %s", tag.name, (char *)tag.data);
						////increase and limit mtag_index
						//mtag_index = (mtag_index + 1) % NUMBER_OF_TAGS;
					}
					else if (tag.type == FMOD_TAGTYPE_FMOD)
					{

						//whe a song changes, the sample rate might also change
						float frequency = *((float *)tag.data);
						mresult = pChannel[chosenStream]->setFrequency(frequency);
						errorcheck(mresult);
					}
				}

				//Get system settings
				mresult = pChannel[chosenStream]->getPaused(&is_paused);
				errorcheck(mresult);
				mresult = pChannel[chosenStream]->isPlaying(&is_playing);
				errorcheck(mresult);
				mresult = pChannel[chosenStream]->getPosition(&position, FMOD_TIMEUNIT_MS);
				errorcheck(mresult);
				mresult = pChannel[chosenStream]->setMute(is_starving);
				errorcheck(mresult);


			}
			else
			{
				//This may fail if the stream isn't ready yet, so don't check the error code
				msystem->playSound(pSound[chosenStream], 0, false, &pChannel[chosenStream]);
			}
		}

		//Print to screen based on what stream is selected
		if (chosenStream < 4)
		{
			//Check stream/sound settings
			if (mopenstate == FMOD_OPENSTATE_CONNECTING)
				current_state = "Connecting...";
			else if (mopenstate == FMOD_OPENSTATE_BUFFERING)
				current_state = "Buffering...";
			else if (is_paused)
				current_state = "Paused...";
			else if (is_playing)
				current_state = "Playing...";

			print_text("==============================================================");
			print_text("Media Fundamentals Project 3");
			print_text("==============================================================");
			print_text("");
			//TODO: 
			//Print sound info/settings
			print_text("Time: %02d:%02d:%02d", position / 1000 / 60, position / 1000 % 60, position / 10 % 100);
			print_text("Current state: %s %s", current_state, is_starving ? "(STARVING)" : "");
			print_text("Buffer percentage: %d", percentage);
			print_text("");

			print_text("Title: %s", title[chosenStream]);
			print_text("");
			print_text("Artist: %s", artist[chosenStream]);
			print_text("");
			print_text("Genre: %s", genre[chosenStream]);
			print_text("");

			print_text("Press ESC to quit");

			//Needed for print_text
			end_text();
		}

		else if (chosenStream == 4)
		{
			print_text("==============================================================");
			print_text("Media Fundamentals Project 3");
			print_text("==============================================================");
			print_text("");
			print_text("");
			print_text("Press ESC to quit");
		}

		Sleep(50);

	}

	for (int i = 0; i < 3; i++)
	{
		if (pChannel[i])
		{
			mresult = pChannel[i]->stop();
			errorcheck(mresult);
		}
	}

	//Wait for sound to finish opening before trying to release it
		start_text();
		ugly_clear_screen();

		print_text("waiting for sound to finish opening before trying to release it..!");

		end_text();

		do
		{
			mresult = msystem->update();
			errorcheck(mresult);
			mresult = pSound[0]->getOpenState(&mopenstate, 0, 0, 0);
			errorcheck(mresult);
		} while (mopenstate != FMOD_OPENSTATE_READY);

		do
		{
			mresult = msystem->update();
			errorcheck(mresult);
			mresult = pSound[1]->getOpenState(&mopenstate, 0, 0, 0);
			errorcheck(mresult);
		} while (mopenstate != FMOD_OPENSTATE_READY);

		do
		{
			mresult = msystem->update();
			errorcheck(mresult);
			mresult = pSound[2]->getOpenState(&mopenstate, 0, 0, 0);
			errorcheck(mresult);
		} while (mopenstate != FMOD_OPENSTATE_READY);

	for (int i = 0; i < 3; i++)
	{

		if (pSound[i])
		{
			pSound[i]->release();
			errorcheck(mresult);
		}
	}



	if (msystem) {
		mresult = msystem->close();
		errorcheck(mresult);
		mresult = msystem->release();
		errorcheck(mresult);
	}

	return 0;

}