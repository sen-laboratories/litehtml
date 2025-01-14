/*
 * Copyright 2019-2020 Haiku Inc.
 * All rights reserved. Distributed under the terms of the BSD 3-clause license.
 * Constributors
 * 2019-2020	Adam Fowler <adamfowleruk@gmail.com>
 * 2025         Gregor Rosenauer <gregor.rosenauer@gmail.com>
 */
#ifndef LITEHTMLVIEW_H
#define LITEHTMLVIEW_H

#include "../../include/litehtml.h"
#include "../../include/litehtml/background.h"
#include "../../include/litehtml/document_container.h"
#include "../../include/litehtml/formatting_context.h"

#include <map>
#include <string>

#include <Url.h>
#include <View.h>

#include <private/netservices2/HttpSession.h>

class BBitmap;

using namespace litehtml;

enum {
	M_HTML_RENDERED = 'hrnd'
};

class LiteHtmlView : public BView, public document_container
{
public:
										//LiteHtmlView(BMessage *archive);
										LiteHtmlView(BRect frame, const char *name);
										//LiteHtmlView(const char *name, uint32 flags, BLayout *layout=NULL);

	virtual								~LiteHtmlView();

    void        						SetContext(formatting_context* ctx);
    void		        				RenderFile(const char* localFilePath);
    void				        		RenderHtml(const BString& htmlText);
    void						        RenderUrl(const BUrl& url);
    void                                RenderUrl(const char* fileOrHttpUrl);
    const BString&                      FetchHttpContent(const BUrl& fileOrHttpUrl);

	virtual uint_ptr			        create_font(const char* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, font_metrics* fm) override;
	virtual void						delete_font(uint_ptr hFont) override;
	virtual int							text_width(const char* text, uint_ptr hFont) override;
	virtual void						draw_text(uint_ptr hdc, const char* text, uint_ptr hFont, web_color color, const position& pos) override;
	virtual int							pt_to_px(int pt) const override;
	virtual int							get_default_font_size() const override;
	virtual const char*	                get_default_font_name() const override;
	virtual void 						load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
	virtual void						get_image_size(const char* src, const char* baseurl, size& sz) override;
    virtual void                        draw_image(uint_ptr hdc, const background_layer& layer, const std::string& url, const std::string& base_url) override;
	virtual void						draw_solid_fill(uint_ptr hdc, const background_layer& layer, const web_color& color) override;

	virtual void						draw_borders(uint_ptr hdc, const borders& borders, const position& draw_pos, bool root) override;
	virtual void 						draw_list_marker(uint_ptr hdc, const list_marker& marker) override;
	virtual std::shared_ptr<element>	create_element(const char *tag_name,
																 const string_map &attributes,
																 const std::shared_ptr<document> &doc) override;
	virtual void						get_media_features(media_features& media) const override;
	//virtual void						get_language(string& language, tstring & culture) const override;
	virtual void 						link(const std::shared_ptr<document> &ptr, const element::ptr& el) override;


	virtual	void						transform_text(string& text, text_transform tt) override;
	virtual void						set_clip(const position& pos, const border_radiuses& bdr_radius) override;
	virtual void						del_clip() override;

    virtual void                    	draw_linear_gradient(uint_ptr hdc, const background_layer& layer, const background_layer::linear_gradient& gradient) override;
    virtual void	                    draw_radial_gradient(uint_ptr hdc, const background_layer& layer, const background_layer::radial_gradient& gradient) override;
    virtual void	                    draw_conic_gradient(uint_ptr hdc, const background_layer& layer, const background_layer::conic_gradient& gradient) override;

	virtual void 						set_caption(const char*) override;
	virtual void						get_client_rect(position& client) const override;
	virtual void 						set_base_url(const char*) override;
	virtual void 						on_anchor_click(const char*, const element::ptr&) override;
    virtual void		        		on_mouse_event(const element::ptr& el, mouse_event event) override;
	virtual void 						set_cursor(const char*) override;
	virtual void 						import_css(string&, const string&, string&) override;
	virtual void 						get_language(string&, string&) const override;

	//BView
	virtual void 						Draw(BRect updateRect) override;
	virtual void						GetPreferredSize(float* width, float* height) override;

protected:
	void								make_url(const char* relativeUrl, const char* baseUrl, BUrl& outUrl);
    const BString&                      FetchLocalContentFromFile(const BUrl& fileOrHttpUrl);
    const BString&                      FetchRemoteContentFromUrl(const BUrl& fileOrHttpUrl);

private:
	formatting_context*	                fContext;
    BPrivate::Network::BHttpSession*    fHttpSession;
	document::ptr				        m_html;
	std::map<uint32 ,BBitmap*>          m_images;
	string					            m_base_url;
	string					            m_url;
};

#endif
