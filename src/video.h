#ifndef VIDEO_H
#define VIDEO_H

#include "types.h"

extern uint screen_width;
extern uint screen_height;
extern uchar video_mode;

void set_video_mode (uchar mode);
void set40x25mode ();
void set80x25mode ();
void set80x50mode ();
void set320x200mode ();
void set640x480mode ();

#endif /* VIDEO_H */
