/**************************************************************************/
/*  camera_feed.h                                                         */
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

#ifndef CAPTURE_FEED_H
#define CAPTURE_FEED_H

#include "core/io/image.h"
#include "core/math/transform_2d.h"
#include "servers/capture_server.h"
#include "servers/rendering_server.h"

/**
	The camera server is a singleton object that gives access to the various
	camera feeds that can be used as the background for our environment.
**/

class CaptureFeed : public RefCounted {
	GDCLASS(CaptureFeed, RefCounted);

public:
	enum FeedDataType {
		FEED_NOIMAGE, // we don't have an image yet
		FEED_RGB, // our texture will contain a normal RGB texture that can be used directly
	};

private:
	int id; // unique id for this, for internal use in case feeds are removed
	int base_width;
	int base_height;

protected:
	Rect2i geometry; // window geometry
	Vector2i position; // window absolute position
	String wm_class; // WM_CLASS
	String wm_name; // WM_NAME
	FeedDataType datatype; // type of texture data stored
	Transform2D transform; // display transform

	bool active; // only when active do we actually update the camera texture each frame
	RID texture[CaptureServer::FEED_IMAGES]; // texture images needed for this

	static void _bind_methods();

public:
	int get_id() const;
	bool is_active() const;
	void set_active(bool p_is_active);

	String get_wm_class() const;
	String get_wm_name() const;
	Rect2i get_geom() const;
	void set_geom(Rect2i);
	void set_wm_class(String p_class);
	void set_wm_name(String p_name);

	int get_base_width() const;
	int get_base_height() const;

	Vector2i get_position() const;
	void set_position(Vector2i position);

	Transform2D get_transform() const;
	void set_transform(const Transform2D &p_transform);

	RID get_texture(CaptureServer::FeedImage p_which);

	CaptureFeed();
	CaptureFeed(String p_wm_name, String p_wm_class);
	virtual ~CaptureFeed();

	FeedDataType get_datatype() const;
	void set_RGB_img(const Ref<Image> &p_rgb_img);

	virtual bool activate_feed();
	virtual void deactivate_feed();
};

VARIANT_ENUM_CAST(CaptureFeed::FeedDataType);

#endif // CAPTURE_FEED_H
