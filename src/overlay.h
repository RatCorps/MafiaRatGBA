
//{{BLOCK(overlay)

//======================================================================
//
//	overlay, 128x128@4, 
//	Transparent color : FF,CC,AA
//	+ palette 256 entries, not compressed
//	+ 256 tiles Metatiled by 2x2 not compressed
//	Total size: 512 + 8192 = 8704
//
//	Time-stamp: 2026-07-23, 23:22:06
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_OVERLAY_H
#define GRIT_OVERLAY_H

#define overlayTilesLen 8192
extern const unsigned int overlayTiles[2048];

#define overlayPalLen 512
extern const unsigned short overlayPal[256];

#endif // GRIT_OVERLAY_H

//}}BLOCK(overlay)
