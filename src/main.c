#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

#include <fileioc.h>
#include <usbdrvce.h>
#include <keypadc.h>
#include <fontlibc.h>
#include <graphx.h>

#define NOTE_STOP -1

// 1 for dongle, 2 for wired
#define INTERFACE 1

struct note {
	uint24_t time;
	bool trackpad;
	uint16_t pulse_high_duration;
	uint16_t pulse_low_duration;
	uint16_t pulse_repeat_count;
};

struct note stop = {
		0,
		0,
		0,
		0,
		0
};

static usb_error_t handle_usb_event(usb_event_t event, void *event_data,
                                    usb_callback_data_t *callback_data) {
	switch(event) {
		case USB_DEVICE_CONNECTED_EVENT: {
			uint8_t buf[256];
			usb_device_t dev;
			usb_error_t err;
			size_t len;

			*(usb_device_t *) callback_data = dev = (usb_device_t) event_data;
			usb_ResetDevice(dev);
			usb_WaitForEvents();

			len = usb_GetConfigurationDescriptorTotalLength(dev, 0);
			err = usb_GetDescriptor(dev, USB_CONFIGURATION_DESCRIPTOR, 0, buf, len, NULL);
			dbg_sprintf(dbgout, "descriptor get error %u\n", err);
			err = usb_SetConfiguration(dev, (usb_configuration_descriptor_t *) buf, len);
			dbg_sprintf(dbgout, "config set error %u\n", err);

			return USB_SUCCESS;
		}
		case USB_DEVICE_DISCONNECTED_EVENT:
			*(usb_device_t*) callback_data = NULL;
			return USB_SUCCESS;
		default:
			return USB_SUCCESS;
	}
}

void playNote(struct note *note, usb_device_t dev, uint8_t interface){
	unsigned char dataBlob[64] = {0x8f,
	                              0x08, //7
	                              0x00, //Trackpad select : 0x01 = left, 0x00 = right
	                              0xff, //LSB Pulse High Duration
	                              0xff, //MSB Pulse High Duration
	                              0xff, //LSB Pulse Low Duration
	                              0xff, //MSB Pulse Low Duration
	                              0xff, //LSB Pulse repeat count
	                              0x04, //MSB Pulse repeat count
	                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	usb_control_setup_t setup = {0x21, 9, 0x300, 0, sizeof(dataBlob)};

	memcpy(&dataBlob[2], &note->trackpad, sizeof(*note) - sizeof(note->time));
	setup.wIndex = interface;

	dbg_sprintf(dbgout, "playing note on haptic %u\n", note->trackpad);

	usb_DefaultControlTransfer(dev, &setup, dataBlob, 10, NULL);
}

void main(void) {
	fontlib_font_t *font;
	ti_var_t slot;
	usb_device_t dev = NULL;
	uint24_t notes;
	uint24_t current_note;
	struct note *note;

	gfx_Begin();
	gfx_FillScreen(gfx_black);

	font = fontlib_GetFontByIndex("GOHUFONT", 1);
	if(font) {
		fontlib_SetFont(font, 0);
	} else {
		goto exit;
	}
	fontlib_SetWindowFullScreen();
	fontlib_SetCursorPosition(0, 0);
	fontlib_SetTransparency(false);
	fontlib_SetBackgroundColor(gfx_black);
	fontlib_SetForegroundColor(gfx_white);
	fontlib_SetNewlineOptions(0);

    ti_CloseAll();
    slot = ti_Open("SONG", "r");
    notes = ti_GetSize(slot) / sizeof(*note);
    note = ti_GetDataPtr(slot);

    dbg_sprintf(dbgout, "got %u notes\n", notes);
    dbg_sprintf(dbgout, "note starts at: %u\n", (uint24_t)note->time);

    usb_Init(handle_usb_event, &dev, NULL, USB_DEFAULT_INIT_FLAGS);

    while(!dev) {
    	kb_Scan();
    	if(kb_IsDown(kb_KeyClear)) goto exit;
    	usb_HandleEvents();
    }

    dbg_sprintf(dbgout, "got device\n");

    timer_Control = TIMER1_DISABLE;
    timer_1_Counter = 0;
	timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

	dbg_Debugger();

	for(current_note = 0; current_note < notes; current_note++) {
		char tmpstr[100];
		kb_Scan();
		if(kb_IsDown(kb_KeyClear)) break;
		// Wait until time for next note
		dbg_sprintf(dbgout, "playing note %u\n", current_note);
		dbg_sprintf(dbgout, "time: %u\n", (uint24_t)note->time);
		if(note->pulse_repeat_count) {
			sprintf(tmpstr, "playing note %u (%5u @ %5s)\n", current_note,
			        note->pulse_high_duration, note->trackpad ? "left" : "right", note->pulse_repeat_count);
			fontlib_SetCursorPosition(0, fontlib_GetCurrentFontHeight() * note->trackpad);
			fontlib_DrawString(tmpstr);
		}
		while(timer_1_Counter < note->time);
		playNote(note, dev, INTERFACE);
		note++;
		usb_HandleEvents();
		if(!dev) goto exit;
	}

	stop.trackpad = 0;
	playNote(&stop, dev, INTERFACE);
	stop.trackpad = 1;
	playNote(&stop, dev, INTERFACE);

	exit:
	gfx_End();
	ti_CloseAll();
	usb_Cleanup();
}
