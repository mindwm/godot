/**************************************************************************/
/*  camera_win.cpp                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/composite.h>
#include <core/error/error_macros.h>
#include "capture_x11.h"

xcb_atom_t ATOM_UTF8_STRING;
xcb_atom_t ATOM_STRING;
xcb_atom_t ATOM_TEXT;
xcb_atom_t ATOM_COMPOUND_TEXT;
xcb_atom_t ATOM_WM_NAME;
xcb_atom_t ATOM_WM_CLASS;
xcb_atom_t ATOM__NET_WM_NAME;
xcb_atom_t ATOM__NET_SUPPORTING_WM_CHECK;
xcb_atom_t ATOM__NET_CLIENT_LIST;

//////////////////////////////////////////////////////////////////////////
// CameraFeedWindows - Subclass for our camera feed on windows


xcb_get_property_reply_t *xcomp_property_sync(xcb_connection_t *conn,
					      xcb_window_t win, xcb_atom_t atom)
{
	if (atom == XCB_ATOM_NONE)
		return NULL;

	xcb_generic_error_t *err = NULL;
	// Read properties up to 4096*4 bytes
	xcb_get_property_cookie_t prop_cookie =
		xcb_get_property(conn, 0, win, atom, 0, 0, 4096);
	xcb_get_property_reply_t *prop =
		xcb_get_property_reply(conn, prop_cookie, &err);
	if (err != NULL || xcb_get_property_value_length(prop) == 0) {
		free(prop);
		return NULL;
	}

	return prop;
}


bool xcomp_check_ewmh(xcb_connection_t *conn, xcb_window_t root)
{
	xcb_get_property_reply_t *check =
		xcomp_property_sync(conn, root, ATOM__NET_SUPPORTING_WM_CHECK);
	if (!check)
		return false;

	xcb_window_t ewmh_window =
		((xcb_window_t *)xcb_get_property_value(check))[0];
	free(check);

	xcb_get_property_reply_t *check2 = xcomp_property_sync(
		conn, ewmh_window, ATOM__NET_SUPPORTING_WM_CHECK);
	if (!check2)
		return false;
	free(check2);

	return true;
}

xcb_screen_t *xcb_get_screen(xcb_connection_t *xcb, int screen)
{
	xcb_screen_iterator_t iter;

	iter = xcb_setup_roots_iterator(xcb_get_setup(xcb));
	for (; iter.rem; --screen, xcb_screen_next(&iter)) {
		if (screen == 0)
			return iter.data;
	}

	return NULL;
}

xcb_atom_t get_atom(xcb_connection_t *conn, const char *name)
{
	xcb_intern_atom_cookie_t atom_c =
		xcb_intern_atom(conn, 1, strlen(name), name);
	xcb_intern_atom_reply_t *atom_r =
		xcb_intern_atom_reply(conn, atom_c, NULL);
	xcb_atom_t a = atom_r->atom;
	free(atom_r);
	return a;
}

void xcomp_gather_atoms(xcb_connection_t *conn)
{
	ATOM_UTF8_STRING = get_atom(conn, "UTF8_STRING");
	ATOM_STRING = get_atom(conn, "STRING");
	ATOM_TEXT = get_atom(conn, "TEXT");
	ATOM_COMPOUND_TEXT = get_atom(conn, "COMPOUND_TEXT");
	ATOM_WM_NAME = get_atom(conn, "WM_NAME");
	ATOM_WM_CLASS = get_atom(conn, "WM_CLASS");
	ATOM__NET_WM_NAME = get_atom(conn, "_NET_WM_NAME");
	ATOM__NET_SUPPORTING_WM_CHECK =
		get_atom(conn, "_NET_SUPPORTING_WM_CHECK");
	ATOM__NET_CLIENT_LIST = get_atom(conn, "_NET_CLIENT_LIST");
}


class CaptureFeedWindows : public CaptureFeed {
private:
protected:
public:
	CaptureFeedWindows();
	virtual ~CaptureFeedWindows();

	bool activate_feed();
	void deactivate_feed();
};

CaptureFeedWindows::CaptureFeedWindows() {
	///@TODO implement this, should store information about our available camera
}

CaptureFeedWindows::~CaptureFeedWindows() {
	// make sure we stop recording if we are!
	if (is_active()) {
		deactivate_feed();
	};

	///@TODO free up anything used by this
};

bool CaptureFeedWindows::activate_feed() {
	///@TODO this should activate our camera and start the process of capturing frames

	return true;
};

///@TODO we should probably have a callback method here that is being called by the
// camera API which provides frames and call back into the CameraServer to update our texture

void CaptureFeedWindows::deactivate_feed() {
	///@TODO this should deactivate our camera and stop the process of capturing frames
}

//////////////////////////////////////////////////////////////////////////
// CameraWindows - Subclass for our camera server on windows

void CaptureX11::add_active_windows() {
	xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
	for (; screen_iter.rem > 0; xcb_screen_next(&screen_iter)) {
		xcb_generic_error_t *err = NULL;
		// Read properties up to 4096*4 bytes
		xcb_get_property_cookie_t cl_list_cookie =
			xcb_get_property(conn, 0, screen_iter.data->root,
					 ATOM__NET_CLIENT_LIST, 0, 0, 4096);
		xcb_get_property_reply_t *cl_list =
			xcb_get_property_reply(conn, cl_list_cookie, &err);
		if (err != NULL) {
			free(cl_list);
			ERR_PRINT("failed to list windows");
		} else {
  		uint32_t len = xcb_get_property_value_length(cl_list) /
  			       sizeof(xcb_window_t);
  		for (uint32_t i = 0; i < len; i++){
      	Ref<CaptureFeed> newfeed;
      	newfeed.instantiate();
				xcb_window_t cwin = (((xcb_window_t *)xcb_get_property_value( cl_list))[i]);
				xcb_get_property_reply_t *name = xcomp_property_sync(conn, cwin, ATOM__NET_WM_NAME);
				if (name) {
					char buf[1024];
					const char *data = (const char *)xcb_get_property_value(name);
					strcpy(buf, data);
					WARN_PRINT(vformat("%s", buf));
      		newfeed->set_name(buf);
				} else {
					ERR_PRINT(vformat("cannot get name for %d", cwin));
      		newfeed->set_name(vformat("noname_%d", i));
				}
      	add_feed(newfeed);
  		}
		}
	}

}

void CaptureX11::xcomposite_load() {
	disp = XOpenDisplay(NULL);
	conn = XGetXCBConnection(disp);
	ERR_FAIL_COND_MSG(xcb_connection_has_error(conn), "failed opening display");

	const xcb_query_extension_reply_t *xcomp_ext =
		xcb_get_extension_data(conn, &xcb_composite_id);
	ERR_FAIL_COND_MSG(!xcomp_ext->present, "Xcomposite extension not supported");

	xcb_composite_query_version_cookie_t version_cookie =
		xcb_composite_query_version(conn, 0, 2);
	xcb_composite_query_version_reply_t *version =
		xcb_composite_query_version_reply(conn, version_cookie, NULL);
	ERR_FAIL_COND_MSG(version->major_version == 0 && version->minor_version < 2,
			vformat("Xcomposite extension is too old: %d.%d < 0.2", version->major_version, version->minor_version));

	free(version);

	// Must be done before other helpers called.
	xcomp_gather_atoms(conn);

	xcb_screen_t *screen_default =
		xcb_get_screen(conn, DefaultScreen(disp));
	ERR_FAIL_COND_MSG(!screen_default || !xcomp_check_ewmh(conn, screen_default->root), "window manager does not support Extended Window Manager Hints (EWMH).\nXComposite capture disabled.");

}

CaptureX11::CaptureX11() {
	xcomposite_load();
	add_active_windows();

	// need to add something that will react to devices being connected/removed...
};
