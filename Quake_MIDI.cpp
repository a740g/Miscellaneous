/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*
MIDI music support for Quake. Too lazy to put in the Quake CD!!! ;)
Copyright (c) Samuel Gomes (Blade), 2001-2002
mailto: blade_go@hotmail.com
*/

/*
Notes:
We know that there are 1 data + 10 audio tracks on the Quake CD.
Therefore, we map each audio track to a different bunch of MIDI
files. For example audio track 2 (the 1st audio track) could be
mapped to MIDI file from music0t2.mid to music7t2.MID, audio track 3
can be mapped to music0t3.MID to music7t3.MID and so on. This is just
an example. MIDI files are named as MUS_xTy.MID, where x is a +ve
integer from 0 specifying the member no. of the MIDI file mapped to
a particular Audio CD track; y is the actual Audio CD track number.
All MIDI files will be stored in the "music" folder under the "id1"
folder (preferably in a PAK file). The MIDI code has been implemented
using the Win32 Stream MIDI functions which enable us to load MIDI
music directly from PAK files. ;)
*/


#include <windows.h>
#include "quakedef.h"

#pragma comment(exestr, "$Id: "__FILE__", v"GFRAME_VERSION" "__TIMESTAMP__" blade Exp $")

/* ------------- Quake specific stuff -------------- */
/*
No of MIDI files mapped to a single CD Audio track.
Can range from 0 to 9 only. But then if you change this, you may
have to add extra MIDI files to the groups.
*/
#define MIDI_GROUP_MEMBERS 3
/* No. of audio tracks */
#define CD_TRACKS 10
/* Lowest audio track */
#define CD_MIN_TRACK 2
/* Highest audio track */
#define CD_MAX_TRACK 11


/* Main application window */
extern cvar_t bgmvolume;
extern HWND mainwindow;

static BOOL MIDI_Enabled = FALSE;
static char MIDI_FileName[_MAX_PATH];
static float MIDI_Volume = 1;
static int MIDI_CDTrack = 0;
static char scratch[_MAX_PATH];



/* -------------- Win32 Stream MIDI stuff -------------- */

#define MIDI_TRACKS 32

/* A MIDI file */
typedef struct {
   int divisions;                      /* number of ticks per quarter note */
   struct {
      unsigned char *data;             /* MIDI message stream */
      int len;                         /* length of the track data */
   } track[MIDI_TRACKS]; 
} MIDI;

static MIDI mididata;
static BOOL MusicLoaded = FALSE;
static BOOL MusicLoop = FALSE;

static HMIDISTRM hMidiStream;
static MIDIEVENT *MidiEvents[MIDI_TRACKS];
static MIDIHDR MidiStreamHdr;
static MIDIEVENT *NewEvents;
static int NewSize;
static int NewPos;
static int BytesRecorded[MIDI_TRACKS];
static int BufferSize[MIDI_TRACKS];
static int CurrentTrack;
static int CurrentPos;

// Some strings of bytes used in the MIDI format
static BYTE midikey[] =
	{0x00,0xff,0x59,0x02,0x00,0x00};        // C major
static BYTE miditempo[] =
	{0x00,0xff,0x51,0x03,0x09,0xa3,0x1a};   // uS/qnote
static BYTE midihdr[] =
	{'M','T','h','d',0,0,0,6,0,1,0,0,0,0};  // header (length 6, format 1)
static BYTE trackhdr[] =
	{'M','T','r','k'};                      // track header


//
// ReadLength()
//
// Reads the length of a chunk in a midi buffer, advancing the pointer
// 4 bytes, bigendian
//
// Passed a pointer to the pointer to a MIDI buffer
// Returns the chunk length at the pointer position
//
static size_t ReadLength(BYTE **mid) {
  BYTE *midptr = *mid;
  size_t length = (*midptr++) << 24;
  length += (*midptr++) << 16;
  length += (*midptr++) << 8;
  length += *midptr++;
  *mid = midptr;

  return length;
}


//
// MidiToMIDI()
//
// Convert an in-memory copy of a MIDI format 0 or 1 file to 
// our custom MIDI structure, that is valid or has been zeroed
//
// Passed a pointer to a memory buffer with MIDI format music in it
//
// Returns TRUE if successful, FALSE if the buffer is not MIDI format
//
static BOOL ConvertMIDI(BYTE *mid) {
	int i;
	int ntracks;

	// read the midi header

	if (memcmp(mid, midihdr, 4))
		return FALSE;

	mididata.divisions = (mid[12] << 8) + mid[13];
	ntracks = (mid[10] << 8) + mid[11];

	if (ntracks >= MIDI_TRACKS)
		return FALSE;

	mid += 4;
	mid += ReadLength(&mid);		// seek past header

	// now read each track

	for (i = 0; i < ntracks; i++) {
		while (memcmp(mid, trackhdr, 4)) {	// simply skip non-track data
			mid += 4;
			mid += ReadLength(&mid);
		}
		mid += 4;
		mididata.track[i].len = ReadLength(&mid);  // get length, move mid past it

		// read a track
		mididata.track[i].data = realloc(mididata.track[i].data, mididata.track[i].len);
		memcpy(mididata.track[i].data, mid, mididata.track[i].len);
		mid += mididata.track[i].len;
	}
	for (; i<MIDI_TRACKS; i++) {
		if (mididata.track[i].len) {
			free(mididata.track[i].data);
			mididata.track[i].data = NULL;
			mididata.track[i].len = 0;
		}
	}
	
	return TRUE;
}


static int GetVL(void) {
	int l = 0;
	BYTE c;
	
	for (;;) {
		c = mididata.track[CurrentTrack].data[CurrentPos];
		CurrentPos++;
		l += (c & 0x7f);
		if (!(c & 0x80)) 
			return l;
		l<<=7;
	}
}


static void AddEvent(DWORD at, DWORD type, BYTE event, BYTE a, BYTE b) {
	MIDIEVENT *CurEvent;

	if ((BytesRecorded[CurrentTrack] + (int)sizeof(MIDIEVENT)) >= BufferSize[CurrentTrack]) {
		BufferSize[CurrentTrack] += 100 * sizeof(MIDIEVENT);
		MidiEvents[CurrentTrack] = realloc(MidiEvents[CurrentTrack], BufferSize[CurrentTrack]);
	}
	CurEvent = (MIDIEVENT *)((byte *)MidiEvents[CurrentTrack] + BytesRecorded[CurrentTrack]);
	memset(CurEvent, 0, sizeof(MIDIEVENT));
	CurEvent->dwDeltaTime = at;
	CurEvent->dwEvent = event + (a << 8) + (b << 16) + (type << 24);
	BytesRecorded[CurrentTrack] += 3 * sizeof(DWORD);
}


static void TrackToStream(void) {
	DWORD atime, len;
	BYTE event, type, a, b, c;
	BYTE laststatus, lastchan;

	CurrentPos = 0;
	laststatus = 0;
	lastchan = 0;
	atime = 0;
	for (;;) {
		if (CurrentPos >= mididata.track[CurrentTrack].len)
			return;
		atime += GetVL();
		event = mididata.track[CurrentTrack].data[CurrentPos];
		CurrentPos++;
		if (event == 0xF0 || event == 0xF7) {	/* SysEx event */
			len = GetVL();
			CurrentPos += len;
		}
		else if (event == 0xFF) {	/* Meta event */
			type = mididata.track[CurrentTrack].data[CurrentPos];
			CurrentPos++;
			len = GetVL();
			
			switch(type) {
				case 0x2f:
					return;
				case 0x51: /* Tempo */
					a = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					b = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					c = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					AddEvent(atime, MEVT_TEMPO, c, b, a);
					break;
				default:
					CurrentPos += len;
					break;
			}
		}
		else {
			a = event;
			if (a & 0x80) {	/* status byte */
				lastchan = a & 0x0F;
				laststatus = (a >> 4) & 0x07;
				a = mididata.track[CurrentTrack].data[CurrentPos];
				CurrentPos++;
				a &= 0x7F;
			}
			switch (laststatus) {
				case 0: /* Note off */
					b = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					b &= 0x7F;
					AddEvent(atime, MEVT_SHORTMSG, (byte)((laststatus << 4) + lastchan + 0x80), a, b);
					break;

				case 1: /* Note on */
					b = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					b &= 0x7F;
					AddEvent(atime, MEVT_SHORTMSG, (byte)((laststatus << 4) + lastchan + 0x80), a, b);
					break;

				case 2: /* Key Pressure */
					b = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					b &= 0x7F;
					AddEvent(atime, MEVT_SHORTMSG, (byte)((laststatus << 4) + lastchan + 0x80), a, b);
					break;

				case 3: /* Control change */
					b = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					b &= 0x7F;
					AddEvent(atime, MEVT_SHORTMSG, (byte)((laststatus << 4) + lastchan + 0x80), a, b);
					break;

				case 4: /* Program change */
					a &= 0x7f;
					AddEvent(atime, MEVT_SHORTMSG, (byte)((laststatus << 4) + lastchan + 0x80), a, 0);
					break;

				case 5: /* Channel pressure */
					a &= 0x7f;
					AddEvent(atime, MEVT_SHORTMSG, (byte)((laststatus << 4) + lastchan + 0x80), a, 0);
					break;

				case 6: /* Pitch wheel */
					b = mididata.track[CurrentTrack].data[CurrentPos];
					CurrentPos++;
					b &= 0x7F;
					AddEvent(atime, MEVT_SHORTMSG, (byte)((laststatus << 4) + lastchan + 0x80), a, b);
					break;

				default: 
					break;
			}
		}
	}
}

static void BlockOut(void) {
	MMRESULT err;
	int BlockSize;

	if ((MusicLoaded) && (NewEvents)) {
		if (NewPos >= NewSize) {
			if (MusicLoop) {
				NewPos = 0;
			}
			else {
				return;
			}
		}
		BlockSize = (NewSize - NewPos);
		if (BlockSize > 36000)
			BlockSize = 36000;
		MidiStreamHdr.lpData = (void *)((byte *)NewEvents + NewPos);
		NewPos += BlockSize;
		MidiStreamHdr.dwBufferLength = BlockSize;
		MidiStreamHdr.dwBytesRecorded = BlockSize;
		MidiStreamHdr.dwFlags = 0;
		err = midiOutPrepareHeader((HMIDIOUT)hMidiStream, &MidiStreamHdr, sizeof(MIDIHDR));
		if (err != MMSYSERR_NOERROR)
			return;
		err = midiStreamOut(hMidiStream, &MidiStreamHdr, sizeof(MIDIHDR));
	}
}


static void MIDItoStream(void) {
	int BufferPos[MIDI_TRACKS];
	MIDIEVENT *CurEvent;
	MIDIEVENT *NewEvent;
	int lTime;
	int Dummy;
	int Track;

	if (!hMidiStream)
		return;
	NewSize=0;
	for (CurrentTrack = 0; CurrentTrack < MIDI_TRACKS; CurrentTrack++) {
		if (MidiEvents[CurrentTrack]) {
			free(MidiEvents[CurrentTrack]);
			MidiEvents[CurrentTrack] = NULL;
		}
		BytesRecorded[CurrentTrack] = 0;
		BufferSize[CurrentTrack] = 0;
		TrackToStream();
		NewSize += BytesRecorded[CurrentTrack];
		BufferPos[CurrentTrack] = 0;
	}
	NewEvents = realloc(NewEvents, NewSize);
	if (NewEvents) {
		NewPos=0;
		for (;;) {
			lTime=INT_MAX;
			Track = -1;
			for (CurrentTrack = MIDI_TRACKS - 1; CurrentTrack >= 0; CurrentTrack--) {
				if ((BytesRecorded[CurrentTrack] > 0) && (BufferPos[CurrentTrack] < BytesRecorded[CurrentTrack]))
					CurEvent = (MIDIEVENT *)((byte *)MidiEvents[CurrentTrack] + BufferPos[CurrentTrack]);
				else 
					continue;
				if ((int)CurEvent->dwDeltaTime <= lTime) {
					lTime = CurEvent->dwDeltaTime;
					Track = CurrentTrack;
				}
			}
			if (Track == -1)
				break;
			else {
				CurEvent = (MIDIEVENT *)((byte *)MidiEvents[Track] + BufferPos[Track]);
				BufferPos[Track] += 12;
				NewEvent = (MIDIEVENT *)((byte *)NewEvents + NewPos);
				memcpy(NewEvent, CurEvent, 12);
				NewPos += 12;
			}
		}
		NewPos = 0;
		lTime = 0;
		while (NewPos < NewSize) {
			NewEvent = (MIDIEVENT *)((byte *)NewEvents + NewPos);
			Dummy = NewEvent->dwDeltaTime;
			NewEvent->dwDeltaTime -= lTime;
			lTime = Dummy;
			NewPos += 12;
		}
		NewPos = 0;
		MusicLoaded = TRUE;
		BlockOut();
	}
	for (CurrentTrack = 0; CurrentTrack < MIDI_TRACKS; CurrentTrack++) {
		if (MidiEvents[CurrentTrack]) {
			free(MidiEvents[CurrentTrack]);
			MidiEvents[CurrentTrack] = NULL;
		}
	}
}


static void CALLBACK MidiProc(HMIDIIN hMidi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
	switch (uMsg) {
		case MOM_DONE:
			BlockOut();
			break;
		default:
			break;
	}
}


static void Music_Play(BOOL looping) {
	if (hMidiStream) {
		MusicLoop = looping;
		midiStreamRestart(hMidiStream);
    }
}

static void Music_Pause(void) {
    if (hMidiStream) midiStreamPause(hMidiStream);
}

static void Music_Resume(void) {
	if (hMidiStream) midiStreamRestart(hMidiStream);
}

static void Music_Stop(void) {
    if (!hMidiStream) return;

    midiStreamStop(hMidiStream);
    midiOutReset((HMIDIOUT)hMidiStream);
}

static void Music_Unregister(void) {
    if (!hMidiStream) return;
    
	MusicLoaded = FALSE;
    midiStreamStop(hMidiStream);
    midiOutReset((HMIDIOUT)hMidiStream);
    midiStreamClose(hMidiStream);
	hMidiStream = 0;
}

static BOOL Music_Register(BYTE *data) {
	MMRESULT merr;
	MIDIPROPTIMEDIV mptd;
	UINT MidiDevice = MIDI_MAPPER;

	if (!ConvertMIDI(data)) return FALSE;

    memset(&MidiStreamHdr, 0, sizeof(MIDIHDR));
    merr = midiStreamOpen(&hMidiStream, &MidiDevice, 1, (DWORD)&MidiProc, 0, CALLBACK_FUNCTION);
    if (merr != MMSYSERR_NOERROR) hMidiStream = 0;
    if (hMidiStream == 0) return FALSE;
	mptd.cbStruct = sizeof(MIDIPROPTIMEDIV);
    mptd.dwTimeDiv = mididata.divisions;
    merr = midiStreamProperty(hMidiStream,(LPBYTE)&mptd,MIDIPROP_SET | MIDIPROP_TIMEDIV);
    MIDItoStream();
	MusicLoaded = TRUE;

	return TRUE;
}

static void Music_Done(void) {
	int i;

	Music_Unregister();

	for (i = 0; i < MIDI_TRACKS; i++) {
		if (mididata.track[i].data != NULL && mididata.track[i].len) {
			free(mididata.track[i].data);
			mididata.track[i].data = NULL;
			mididata.track[i].len = 0;
		}
		if (MidiEvents[i]) {
			free(MidiEvents[i]);
			MidiEvents[i] = NULL;
		}
	}
	if (NewEvents) {
		free(NewEvents);
		NewEvents = NULL;
	}
}

static void Music_Init(void) {
	int i;

	for (i = 0; i < MIDI_TRACKS; i++) {
		mididata.track[i].data = NULL;
		mididata.track[i].len = 0;
		MidiEvents[i] = NULL;
	}
	NewEvents = NULL;
	hMidiStream = 0;
}


/*------------------- Quake MIDI stuff -----------------*/
/*
Maps a Audio CD track to a MIDI file name. Taking care to 'clip'
where necessary.
*/
static char *MIDI_GetNameForAudioCDTrack(int *pTrack) {
	int i, track;

	if (pTrack != NULL)
		track = *pTrack;
	else
		track = CD_MIN_TRACK - 1;	/* random music */

	/* Check track number */
	if (track < CD_MIN_TRACK || track > CD_MAX_TRACK) {
		track = (rand() % CD_TRACKS) + CD_MIN_TRACK;
		if (pTrack != NULL) *pTrack = track;
	}

	/* This generates a random no. < MIDI_GROUP_MEMBERS */
	i = rand() % MIDI_GROUP_MEMBERS;

	sprintf(scratch, "music/mus_%it%i.mid", i, track);

	return scratch;
}

/* Plays a MIDI file */
BOOL MIDI_Play(int iTrack, BOOL bLooping) {
    BYTE *buf;
	BYTE stackbuf[0xFFFF];

	if (!MIDI_Enabled) return FALSE;

	strcpy(MIDI_FileName, MIDI_GetNameForAudioCDTrack(&iTrack));

	/* Show loading info only if track has changed */
	if (iTrack != MIDI_CDTrack) {
		if (bLooping)
			Con_Printf("Looping \"%s\" (track %i)...", MIDI_FileName, iTrack);
		else
			Con_Printf("Playing \"%s\" (track %i)...", MIDI_FileName, iTrack);
	}

	buf = COM_LoadStackFile(MIDI_FileName, stackbuf, sizeof(stackbuf));

	if (!buf) {
		if (iTrack != MIDI_CDTrack) Con_Printf("load failed!\n");
		return FALSE;
	}

	Music_Unregister();

	if (!Music_Register(buf)) {
		if (iTrack != MIDI_CDTrack) Con_Printf("registration failed!\n");
		return FALSE;
	}

	if (MIDI_Volume) {
		Music_Play(bLooping);
	}
	else {
		Music_Play(bLooping);
		Music_Pause();
	}

	if (iTrack != MIDI_CDTrack) Con_Printf("OK.\n");

	MIDI_CDTrack = iTrack;

    return TRUE;
}

/* Pauses MIDI playback */
void MIDI_Pause(void) {
	if (!MIDI_Enabled) return;

	Music_Pause();
}

/* Resumes MIDI playback */
void MIDI_Resume(void) {
	if (!MIDI_Enabled || !MIDI_Volume) return;

	Music_Resume();
}

/* Stops MIDI playback */
void MIDI_Stop(void) {
	if (!MIDI_Enabled) return;

	Music_Stop();
	Music_Unregister();
	MIDI_CDTrack = 0;
}

static void MIDI_f(void) {
	char	*command;

	if (!MIDI_Enabled) return;

	if (Cmd_Argc() < 2)
		return;

	command = Cmd_Argv (1);

	if (stricmp(command, "play") == 0) {
		if (!MIDI_Play(atoi(Cmd_Argv(2)), FALSE)) {
			Con_Printf("MIDI_f: MIDI_Play() failed!\n");
		}
		return;
	}

	if (stricmp(command, "loop") == 0) {
		if (!MIDI_Play(atoi(Cmd_Argv(2)), TRUE)) {
			Con_Printf("MIDI_f: MIDI_Play() failed!\n");
		}
		return;
	}

	if (stricmp(command, "stop") == 0) {
		MIDI_Stop();
		return;
	}

	if (stricmp(command, "pause") == 0) {
		MIDI_Pause();
		return;
	}

	if (stricmp(command, "resume") == 0) {
		MIDI_Resume();
		return;
	}

	if (stricmp(command, "info") == 0) {
		if (MIDI_Volume && MIDI_CDTrack >= CD_MIN_TRACK)
			Con_Printf("Currently %s \"%s\" (track %i).\n", MusicLoop ? "looping" : "playing", MIDI_FileName, MIDI_CDTrack);
		else
			Con_Printf("MIDI music playback suspended.\n");
		return;
	}
}

void MIDI_Update(void) {
	if (!MIDI_Enabled) return;

	if (bgmvolume.value != MIDI_Volume) {
		if (MIDI_Volume) {
			Cvar_SetValue ("bgmvolume", 0.0);
			MIDI_Volume = bgmvolume.value;
			MIDI_Pause();
		}
		else {
			Cvar_SetValue ("bgmvolume", 1.0);
			MIDI_Volume = bgmvolume.value;
			MIDI_Resume();
		}
	}
}

BOOL MIDI_Init(void) {
	if (MIDI_Enabled)
		return FALSE;

	if (cls.state == ca_dedicated)
		return FALSE;

	if (!COM_CheckParm("-nocdaudio"))
		return FALSE;

	/* We emulate the required CD console commands */
	Cmd_AddCommand("cd", MIDI_f);

	Music_Init();

	MIDI_Enabled = TRUE;
	MIDI_CDTrack = 0;

	Con_Printf("MIDI music enabled.\n");

	return TRUE;
}

void MIDI_Done(void) {
	if (!MIDI_Enabled) return;

	MIDI_Stop();
	Music_Done();

	MIDI_CDTrack = 0;
	MIDI_Enabled = FALSE;
}
