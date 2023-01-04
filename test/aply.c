
#include <alsa/asoundlib.h>
#include <stdio.h>

//  amidi -p hw:1,0,0 -S "f0 7e 7f 09 01 f7"
static const  char*          port   = "hw:1,0,0";
static const  char*          dev    = "/dev/midi1";
static const  unsigned char  data[] = {0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7};  // Glob SysEx: GM On

int  main (int argc,  char** argv)
{
	FILE*  fh = fopen(dev, "wb");

	fwrite(data, 1, sizeof(data), fh);

	fclose(fh);

	return 0;
}

int  main2 (int argc,  char** argv)
{
	static  snd_rawmidi_t*  input;
	static  snd_rawmidi_t*  output;

	int err;

	if ((err = snd_rawmidi_open(&input, &output, port, SND_RAWMIDI_NONBLOCK)) < 0) {
		printf("cannot open port \"%s\": %s", port, snd_strerror(err));
		goto _exit;
	}

	if ((err = snd_rawmidi_nonblock(output, 0)) < 0) {
		printf("cannot set blocking mode: %s", snd_strerror(err));
		goto _exit;
	}

	if ((err = snd_rawmidi_write(output, data, sizeof(data))) < 0) {
		printf("cannot send data: %s", snd_strerror(err));
		goto _exit;
	}

_exit:
	snd_rawmidi_close(output);
	return 0;
}
