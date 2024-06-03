/**************************************************************************/
/*  camera_feed.cpp                                                       */
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

#include "capture_feed.h"

#include "servers/rendering_server.h"

void CaptureFeed::_bind_methods() {
	// The setters prefixed with _ are only exposed so we can have feeds through GDExtension!
	// They should not be called by the end user.

	ClassDB::bind_method(D_METHOD("get_id"), &CaptureFeed::get_id);

	ClassDB::bind_method(D_METHOD("is_active"), &CaptureFeed::is_active);
	ClassDB::bind_method(D_METHOD("set_active", "active"), &CaptureFeed::set_active);

	ClassDB::bind_method(D_METHOD("get_name"), &CaptureFeed::get_name);
	ClassDB::bind_method(D_METHOD("_set_name", "name"), &CaptureFeed::set_name);

	ClassDB::bind_method(D_METHOD("get_position"), &CaptureFeed::get_position);
	ClassDB::bind_method(D_METHOD("_set_position", "position"), &CaptureFeed::set_position);

	// Note, for transform some feeds may override what the user sets (such as ARKit)
	ClassDB::bind_method(D_METHOD("get_transform"), &CaptureFeed::get_transform);
	ClassDB::bind_method(D_METHOD("set_transform", "transform"), &CaptureFeed::set_transform);

	ClassDB::bind_method(D_METHOD("_set_RGB_img", "rgb_img"), &CaptureFeed::set_RGB_img);
	ClassDB::bind_method(D_METHOD("_set_YCbCr_img", "ycbcr_img"), &CaptureFeed::set_YCbCr_img);

	ClassDB::bind_method(D_METHOD("get_datatype"), &CaptureFeed::get_datatype);

	ADD_GROUP("Feed", "feed_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "feed_is_active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "feed_transform"), "set_transform", "get_transform");

	BIND_ENUM_CONSTANT(FEED_NOIMAGE);
	BIND_ENUM_CONSTANT(FEED_RGB);
	BIND_ENUM_CONSTANT(FEED_YCBCR);
	BIND_ENUM_CONSTANT(FEED_YCBCR_SEP);

	BIND_ENUM_CONSTANT(FEED_UNSPECIFIED);
	BIND_ENUM_CONSTANT(FEED_FRONT);
	BIND_ENUM_CONSTANT(FEED_BACK);
}

int CaptureFeed::get_id() const {
	return id;
}

bool CaptureFeed::is_active() const {
	return active;
}

void CaptureFeed::set_active(bool p_is_active) {
	if (p_is_active == active) {
		// all good
	} else if (p_is_active) {
		// attempt to activate this feed
		if (activate_feed()) {
			print_line("Activate " + name);
			active = true;
		}
	} else {
		// just deactivate it
		deactivate_feed();
		print_line("Deactivate " + name);
		active = false;
	}
}

String CaptureFeed::get_name() const {
	return name;
}

void CaptureFeed::set_name(String p_name) {
	name = p_name;
}

int CaptureFeed::get_base_width() const {
	return base_width;
}

int CaptureFeed::get_base_height() const {
	return base_height;
}

CaptureFeed::FeedDataType CaptureFeed::get_datatype() const {
	return datatype;
}

CaptureFeed::FeedPosition CaptureFeed::get_position() const {
	return position;
}

void CaptureFeed::set_position(CaptureFeed::FeedPosition p_position) {
	position = p_position;
}

Transform2D CaptureFeed::get_transform() const {
	return transform;
}

void CaptureFeed::set_transform(const Transform2D &p_transform) {
	transform = p_transform;
}

RID CaptureFeed::get_texture(CaptureServer::FeedImage p_which) {
	return texture[p_which];
}

CaptureFeed::CaptureFeed() {
	// initialize our feed
	id = CaptureServer::get_singleton()->get_free_id();
	base_width = 0;
	base_height = 0;
	name = "???";
	active = false;
	datatype = CaptureFeed::FEED_RGB;
	position = CaptureFeed::FEED_UNSPECIFIED;
	transform = Transform2D(1.0, 0.0, 0.0, -1.0, 0.0, 1.0);
	texture[CaptureServer::FEED_Y_IMAGE] = RenderingServer::get_singleton()->texture_2d_placeholder_create();
	texture[CaptureServer::FEED_CBCR_IMAGE] = RenderingServer::get_singleton()->texture_2d_placeholder_create();
}

CaptureFeed::CaptureFeed(String p_name, FeedPosition p_position) {
	// initialize our feed
	id = CaptureServer::get_singleton()->get_free_id();
	base_width = 0;
	base_height = 0;
	name = p_name;
	active = false;
	datatype = CaptureFeed::FEED_NOIMAGE;
	position = p_position;
	transform = Transform2D(1.0, 0.0, 0.0, -1.0, 0.0, 1.0);
	texture[CaptureServer::FEED_Y_IMAGE] = RenderingServer::get_singleton()->texture_2d_placeholder_create();
	texture[CaptureServer::FEED_CBCR_IMAGE] = RenderingServer::get_singleton()->texture_2d_placeholder_create();
}

CaptureFeed::~CaptureFeed() {
	// Free our textures
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	RenderingServer::get_singleton()->free(texture[CaptureServer::FEED_Y_IMAGE]);
	RenderingServer::get_singleton()->free(texture[CaptureServer::FEED_CBCR_IMAGE]);
}

void CaptureFeed::set_RGB_img(const Ref<Image> &p_rgb_img) {
	ERR_FAIL_COND(p_rgb_img.is_null());
	if (active) {
		int new_width = p_rgb_img->get_width();
		int new_height = p_rgb_img->get_height();

		if ((base_width != new_width) || (base_height != new_height)) {
			// We're assuming here that our camera image doesn't change around formats etc, allocate the whole lot...
			base_width = new_width;
			base_height = new_height;

			RID new_texture = RenderingServer::get_singleton()->texture_2d_create(p_rgb_img);
			RenderingServer::get_singleton()->texture_replace(texture[CaptureServer::FEED_RGBA_IMAGE], new_texture);
		} else {
			RenderingServer::get_singleton()->texture_2d_update(texture[CaptureServer::FEED_RGBA_IMAGE], p_rgb_img);
		}

		datatype = CaptureFeed::FEED_RGB;
	}
}

void CaptureFeed::set_YCbCr_img(const Ref<Image> &p_ycbcr_img) {
	ERR_FAIL_COND(p_ycbcr_img.is_null());
	if (active) {
		int new_width = p_ycbcr_img->get_width();
		int new_height = p_ycbcr_img->get_height();

		if ((base_width != new_width) || (base_height != new_height)) {
			// We're assuming here that our camera image doesn't change around formats etc, allocate the whole lot...
			base_width = new_width;
			base_height = new_height;

			RID new_texture = RenderingServer::get_singleton()->texture_2d_create(p_ycbcr_img);
			RenderingServer::get_singleton()->texture_replace(texture[CaptureServer::FEED_RGBA_IMAGE], new_texture);
		} else {
			RenderingServer::get_singleton()->texture_2d_update(texture[CaptureServer::FEED_RGBA_IMAGE], p_ycbcr_img);
		}

		datatype = CaptureFeed::FEED_YCBCR;
	}
}

void CaptureFeed::set_YCbCr_imgs(const Ref<Image> &p_y_img, const Ref<Image> &p_cbcr_img) {
	ERR_FAIL_COND(p_y_img.is_null());
	ERR_FAIL_COND(p_cbcr_img.is_null());
	if (active) {
		///@TODO investigate whether we can use thirdparty/misc/yuv2rgb.h here to convert our YUV data to RGB, our shader approach is potentially faster though..
		// Wondering about including that into multiple projects, may cause issues.
		// That said, if we convert to RGB, we could enable using texture resources again...

		int new_y_width = p_y_img->get_width();
		int new_y_height = p_y_img->get_height();

		if ((base_width != new_y_width) || (base_height != new_y_height)) {
			// We're assuming here that our camera image doesn't change around formats etc, allocate the whole lot...
			base_width = new_y_width;
			base_height = new_y_height;
			{
				RID new_texture = RenderingServer::get_singleton()->texture_2d_create(p_y_img);
				RenderingServer::get_singleton()->texture_replace(texture[CaptureServer::FEED_Y_IMAGE], new_texture);
			}
			{
				RID new_texture = RenderingServer::get_singleton()->texture_2d_create(p_cbcr_img);
				RenderingServer::get_singleton()->texture_replace(texture[CaptureServer::FEED_CBCR_IMAGE], new_texture);
			}
		} else {
			RenderingServer::get_singleton()->texture_2d_update(texture[CaptureServer::FEED_Y_IMAGE], p_y_img);
			RenderingServer::get_singleton()->texture_2d_update(texture[CaptureServer::FEED_CBCR_IMAGE], p_cbcr_img);
		}

		datatype = CaptureFeed::FEED_YCBCR_SEP;
	}
}

bool CaptureFeed::activate_feed() {
	// nothing to do here
	return true;
}

void CaptureFeed::deactivate_feed() {
	// nothing to do here
}
