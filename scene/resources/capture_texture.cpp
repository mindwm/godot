/**************************************************************************/
/*  capture_texture.cpp                                                   */
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

#include "capture_texture.h"

#include "servers/capture/capture_feed.h"

void CaptureTexture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_capture_feed_id", "feed_id"), &CaptureTexture::set_capture_feed_id);
	ClassDB::bind_method(D_METHOD("get_capture_feed_id"), &CaptureTexture::get_capture_feed_id);

	ClassDB::bind_method(D_METHOD("set_which_feed", "which_feed"), &CaptureTexture::set_which_feed);
	ClassDB::bind_method(D_METHOD("get_which_feed"), &CaptureTexture::get_which_feed);

	ClassDB::bind_method(D_METHOD("set_capture_active", "active"), &CaptureTexture::set_capture_active);
	ClassDB::bind_method(D_METHOD("get_capture_active"), &CaptureTexture::get_capture_active);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "capture_feed_id"), "set_capture_feed_id", "get_capture_feed_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "which_feed"), "set_which_feed", "get_which_feed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "capture_is_active"), "set_capture_active", "get_capture_active");
}

int CaptureTexture::get_width() const {
	Ref<CaptureFeed> feed = CaptureServer::get_singleton()->get_feed_by_id(capture_feed_id);
	if (feed.is_valid()) {
		return feed->get_base_width();
	} else {
		return 0;
	}
}

int CaptureTexture::get_height() const {
	Ref<CaptureFeed> feed = CaptureServer::get_singleton()->get_feed_by_id(capture_feed_id);
	if (feed.is_valid()) {
		return feed->get_base_height();
	} else {
		return 0;
	}
}

bool CaptureTexture::has_alpha() const {
	return false;
}

RID CaptureTexture::get_rid() const {
	Ref<CaptureFeed> feed = CaptureServer::get_singleton()->get_feed_by_id(capture_feed_id);
	if (feed.is_valid()) {
		return feed->get_texture(which_feed);
	} else {
		if (_texture.is_null()) {
			_texture = RenderingServer::get_singleton()->texture_2d_placeholder_create();
		}
		return _texture;
	}
}

Ref<Image> CaptureTexture::get_image() const {
	// not (yet) supported
	return Ref<Image>();
}

void CaptureTexture::set_capture_feed_id(int p_new_id) {
	capture_feed_id = p_new_id;
	notify_property_list_changed();
}

int CaptureTexture::get_capture_feed_id() const {
	return capture_feed_id;
}

void CaptureTexture::set_which_feed(CaptureServer::FeedImage p_which) {
	which_feed = p_which;
	notify_property_list_changed();
}

CaptureServer::FeedImage CaptureTexture::get_which_feed() const {
	return which_feed;
}

void CaptureTexture::set_capture_active(bool p_active) {
	Ref<CaptureFeed> feed = CaptureServer::get_singleton()->get_feed_by_id(capture_feed_id);
	if (feed.is_valid()) {
		feed->set_active(p_active);
		notify_property_list_changed();
	}
}

bool CaptureTexture::get_capture_active() const {
	Ref<CaptureFeed> feed = CaptureServer::get_singleton()->get_feed_by_id(capture_feed_id);
	if (feed.is_valid()) {
		return feed->is_active();
	} else {
		return false;
	}
}

CaptureTexture::CaptureTexture() {}

CaptureTexture::~CaptureTexture() {
	if (_texture.is_valid()) {
		ERR_FAIL_NULL(RenderingServer::get_singleton());
		RenderingServer::get_singleton()->free(_texture);
	}
}
