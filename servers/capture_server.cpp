/**************************************************************************/
/*  camera_server.cpp                                                     */
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

#include "capture_server.h"
#include "core/variant/typed_array.h"
#include "rendering_server.h"
#include "servers/capture/capture_feed.h"

////////////////////////////////////////////////////////
// CaptureServer

CaptureServer::CreateFunc CaptureServer::create_func = nullptr;

void CaptureServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_feed", "index"), &CaptureServer::get_feed);
	ClassDB::bind_method(D_METHOD("get_feed_count"), &CaptureServer::get_feed_count);
	ClassDB::bind_method(D_METHOD("feeds"), &CaptureServer::get_feeds);

	ClassDB::bind_method(D_METHOD("add_feed", "feed"), &CaptureServer::add_feed);
	ClassDB::bind_method(D_METHOD("remove_feed", "feed"), &CaptureServer::remove_feed);

	ClassDB::bind_method(D_METHOD("update", "feed"), &CaptureServer::update_feed);

	ADD_SIGNAL(MethodInfo("capture_feed_added", PropertyInfo(Variant::INT, "id")));
	ADD_SIGNAL(MethodInfo("capture_feed_removed", PropertyInfo(Variant::INT, "id")));

	BIND_ENUM_CONSTANT(FEED_RGBA_IMAGE);
};

CaptureServer *CaptureServer::singleton = nullptr;

CaptureServer *CaptureServer::get_singleton() {
	return singleton;
};

int CaptureServer::get_free_id() {
	bool id_exists = true;
	int newid = 0;

	// find a free id
	while (id_exists) {
		newid++;
		id_exists = false;
		for (int i = 0; i < feeds.size() && !id_exists; i++) {
			if (feeds[i]->get_id() == newid) {
				id_exists = true;
			};
		};
	};

	return newid;
};

int CaptureServer::get_feed_index(int p_id) {
	for (int i = 0; i < feeds.size(); i++) {
		if (feeds[i]->get_id() == p_id) {
			return i;
		};
	};

	return -1;
};

Ref<CaptureFeed> CaptureServer::get_feed_by_id(int p_id) {
	int index = get_feed_index(p_id);

	if (index == -1) {
		return nullptr;
	} else {
		return feeds[index];
	}
};

//void CaptureServer::update_feed(const Ref<CaptureFeed> &p_feed) {
//	feed_texture(p_feed->get_id(), FEED_RGBA_IMAGE);
//}

void CaptureServer::add_feed(const Ref<CaptureFeed> &p_feed) {
	ERR_FAIL_COND(p_feed.is_null());

	// add our feed
	feeds.push_back(p_feed);

	print_verbose("CaptureServer: Registered capture " + p_feed->get_wm_name() + " with ID " + itos(p_feed->get_id()) + " at index " + itos(feeds.size() - 1));

	// let whomever is interested know
	emit_signal(SNAME("capture_feed_added"), p_feed->get_id());
};

void CaptureServer::remove_feed(const Ref<CaptureFeed> &p_feed) {
	for (int i = 0; i < feeds.size(); i++) {
		if (feeds[i] == p_feed) {
			int feed_id = p_feed->get_id();

			print_verbose("CaptureServer: Removed capture " + p_feed->get_wm_name() + " with ID " + itos(feed_id) );

			// remove it from our array, if this results in our feed being unreferenced it will be destroyed
			feeds.remove_at(i);

			// let whomever is interested know
			emit_signal(SNAME("capture_feed_removed"), feed_id);
			return;
		};
	};
};

Ref<CaptureFeed> CaptureServer::get_feed(int p_index) {
	ERR_FAIL_INDEX_V(p_index, feeds.size(), nullptr);

	return feeds[p_index];
};

int CaptureServer::get_feed_count() {
	return feeds.size();
};

TypedArray<CaptureFeed> CaptureServer::get_feeds() {
	TypedArray<CaptureFeed> return_feeds;
	int cc = get_feed_count();
	return_feeds.resize(cc);

	for (int i = 0; i < feeds.size(); i++) {
		return_feeds[i] = get_feed(i);
	};

	return return_feeds;
};

RID CaptureServer::feed_texture(int p_id, CaptureServer::FeedImage p_texture) {
	int index = get_feed_index(p_id);
//	print_line("feed_texture from CaptureServer");
	ERR_FAIL_COND_V(index == -1, RID());

	Ref<CaptureFeed> feed = get_feed(index);

	return feed->get_texture(p_texture);
};

CaptureServer::CaptureServer() {
	singleton = this;
};

CaptureServer::~CaptureServer() {
	singleton = nullptr;
};
