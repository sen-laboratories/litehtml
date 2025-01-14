/*
 * Copyright 2019-2020 Haiku Inc.
 * All rights reserved. Distributed under the terms of the BSD 3-clause license.
 * Constributors
 * 2019-2020	Adam Fowler <adamfowleruk@gmail.com>
 */
#include "container_haiku.h"

#include <string>
#include <iostream>
#include <map>

#include <Bitmap.h>
#include <String.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <TranslationUtils.h>

#include <private/netservices2/HttpSession.h>
#include <private/netservices2/HttpRequest.h>
#include <private/netservices2/HttpResult.h>
#include <private/netservices2/NetServicesDefs.h>

using namespace litehtml;
using namespace BPrivate::Network;

LiteHtmlView::LiteHtmlView(BRect frame, const char *name)
	: BView(frame, name, B_FOLLOW_ALL, B_WILL_DRAW),
	fContext(NULL),
	m_html(NULL),
	m_images(),
	m_base_url(),
	m_url()
{
	SetDrawingMode(B_OP_OVER);
	SetFont(be_plain_font);
}

LiteHtmlView::~LiteHtmlView()
{
}

void
LiteHtmlView::SetContext(formatting_context* ctx)
{
	fContext = ctx;
}

void
LiteHtmlView::RenderFile(const char* localFilePath)
{
	std::cout << "RenderFile" << std::endl;

    const char* html = FetchHttpContent(BUrl(localFilePath));
    if (html != NULL)
    {
        RenderHtml(html);
    }
    else
    {
        std::cout << "RenderUrl aborted, no content received." << std::endl;
    }
}

void
LiteHtmlView::RenderUrl(const char* fileOrHttpUrl)
{
    std::cout << "RenderUrl from string " << fileOrHttpUrl << std::endl;

    BUrl url(fileOrHttpUrl);
    if (!url.IsValid()) {
        // try as file
        url = BUrl(BPath(fileOrHttpUrl));
        if (!url.IsValid()) {
            std::cout << "  Invalid URL: " << fileOrHttpUrl << std::endl;
            return;
        }
    }
    RenderUrl(url);
}

void
LiteHtmlView::RenderUrl(const BUrl& url)
{
	std::cout << "RenderUrl from URL " << url << std::endl;

    const char* html = FetchHttpContent(url);
    if (html != NULL)
    {
        RenderHtml(html);
    }
    else
    {
        std::cout << "RenderUrl aborted, no content received." << std::endl;
    }
}

void
LiteHtmlView::RenderHtml(const BString& htmlText)
{
	std::cout << "RenderHTML" << std::endl;

	// now use this string
	m_html = document::createFromString(htmlText.String(), this);

	if (m_html)
	{
		std::cout << "Successfully read html" << std::endl;
		// success
		// post-parse render operations, if required.
		Invalidate();
	} else {
		std::cout << "Failed to read html" << std::endl;
	}

	// always fire the rendering complete message
	std::cout << "Sending html rendered message: " << M_HTML_RENDERED << std::endl;
    SendNotices(M_HTML_RENDERED,new BMessage(M_HTML_RENDERED));
}

const BString&
LiteHtmlView::FetchHttpContent(const BUrl& fileOrHttpUrl)
{
    std::cout << "FetchHttpContent from URL " << fileOrHttpUrl << std::endl;

    bool isFile;

	if (!fileOrHttpUrl.IsValid())
	{
        std::cout << "  Invalid URL: " << fileOrHttpUrl << std::endl;
        return *(new BString(""));
	} else {
		// Fetch content according to protocol
		BString protocol = fileOrHttpUrl.Protocol();
		// Note: file:// is also supported, but seems heavy weight to use
        if (protocol == "file")
        {
            isFile = true;
		}
        else if (protocol.StartsWith("http"))
        {
            isFile = false;
        }
        else
        {
            std::cout << "Unknown protocol '" << protocol
			          << "' for URL: '" << fileOrHttpUrl << "'" << std::endl;
            return *(new BString(""));
        }
    }
    if (isFile)
    {
        std::cout << "  Loading file from URL " << fileOrHttpUrl << std::endl;
        // Get parent folder for the base url
        BString pathFromUrl;
        pathFromUrl = BUrl::UrlDecode(BString(fileOrHttpUrl.Path()));

        BPath htmlPath(pathFromUrl);
        std::cout << "htmlPath is " << htmlPath.Path() << std::endl;

        BPath parentDir;
        htmlPath.GetParent(&parentDir);
        BString baseUrl(fileOrHttpUrl.Protocol());
        baseUrl << "://" << parentDir.Path() << "/";
        set_base_url(baseUrl);

        BFile htmlFile(htmlPath.Path(), B_READ_ONLY);
        status_t result = htmlFile.InitCheck();
        if (result != B_OK)
        {
            std::cout << "error opening file '" << htmlPath.Path() << "':"
                      << strerror(result) << std::endl;
            return *(new BString(""));
        }

        off_t size;
        htmlFile.GetSize(&size);
        if (size <= 0) {
            std::cout << "error: empty/invalid file '" << fileOrHttpUrl << "':"
              << strerror(result) << std::endl;
            return *(new BString(""));
        }
        else
        {
            char buffer[size+1];
            size_t bytesRead = htmlFile.Read(buffer, size);
            if (bytesRead < 0) {
                std::cout << "error reading from file '" << fileOrHttpUrl << "':"
                  << strerror(result) << std::endl;
                return NULL;
            }
            buffer[size] = '\0';
            htmlFile.Unset();

            return *(new BString(buffer));
        }
    } else {
        std::cout << "  fetching from URL " << fileOrHttpUrl.UrlString() << std::endl;
        BString baseUrl(fileOrHttpUrl.Protocol());
        baseUrl << "://" << fileOrHttpUrl.Host() << "/";
        set_base_url(baseUrl);

        BHttpRequest request(fileOrHttpUrl);
        BHttpSession session;
        try {
            BHttpResult result = session.Execute(std::move(request));
            if (result.Status().code >= 200 && result.Status().code <= 400)
            {
                std::cout << "  Request successful, got "
                          << (! result.Body().text.has_value() ? "empty " : "") << "response "
                          << (  result.Body().text.has_value() ? "with body" : "")
                          << " and status " << result.Status().code << ": " << result.Status().text
                          << std::endl;

                auto htmlBody = new BString(result.Body().text.value_or(""));
                return (*htmlBody);
            }
            else
            {
                std::cout << "HTTP error " << result.Status().code
                          << " reading from URL '" << fileOrHttpUrl << "':"
                          << result.Status().text << std::endl;
                return *(new BString(""));
            }
        } catch (const BPrivate::Network::BNetworkRequestError& err) {
            std::cout << "network error " << err.ErrorCode()
              << " reading from URL '" << fileOrHttpUrl << "':"
              << err.Message() << ", detail: "
              << err.DebugMessage() << std::endl;

            return *(new BString(""));
        }
	}
}

void
LiteHtmlView::Draw(BRect b)
{
	std::cout << "DRAW CALLED" << std::endl;

	BRect bounds(Bounds());
	FillRect(bounds,B_SOLID_LOW);

	// b only part of the window, but we need to draw the whole lot

	if (NULL != m_html) {
		BPoint leftTop = bounds.LeftTop();
		position clip(leftTop.x,leftTop.y,
			bounds.Width(),bounds.Height());
		m_html->render(bounds.Width());
		m_html->draw((uint_ptr) this,0,0,&clip);
	}
	SendNotices(M_HTML_RENDERED,new BMessage(M_HTML_RENDERED));
}

void
LiteHtmlView::GetPreferredSize(float* width,float* height)
{
	if (NULL == m_html)
	{
		BRect bounds(Bounds());
		*width = bounds.Width();
		*height = bounds.Height();
	} else {
		*width = m_html->width();
		*height = m_html->height();
	}
}

uint_ptr
LiteHtmlView::create_font( const char* faceName, int size,
	int weight, litehtml::font_style italic, unsigned int decoration,
	font_metrics* fm )
{
	//std::cout << "create_font" << std::endl;
	string_vector fonts;
	split_string(faceName, fonts, ",");
	trim(fonts[0]);

	uint16 face = B_REGULAR_FACE; // default
	if (italic == font_style_italic)
	{
		face |= B_ITALIC_FACE;
	}
	if (decoration & font_decoration_underline)
	{
		face |= B_UNDERSCORE_FACE;
	}
	if (decoration & font_decoration_linethrough)
	{
		face |= B_STRIKEOUT_FACE;
	}
	// Note: LIGHT, HEAVY, CONDENSED not supported in BeOS R5
#ifdef __HAIKU__
	if(weight >= 0 && weight < 150)			face |= B_LIGHT_FACE;
	else if(weight >= 150 && weight < 250)	face |= B_LIGHT_FACE;
	else if(weight >= 250 && weight < 350)	face |= B_LIGHT_FACE;
	//else if(weight >= 350 && weight < 450)	face |= B_REGULAR_FACE;
	//else if(weight >= 450 && weight < 550)	face |= B_REGULAR_FACE;
	else if(weight >= 550 && weight < 650)	face |= B_CONDENSED_FACE;
#else
	else if(weight >= 550 && weight < 650)	face |= B_BOLD_FACE;
#endif
	else if(weight >= 650 && weight < 750)	face |= B_BOLD_FACE;
#ifndef __HAIKU__
	else if(weight >= 750 && weight < 850)	face |= B_BOLD_FACE;
	else if(weight >= 950)					face |= B_BOLD_FACE;
#else
	else if(weight >= 750 && weight < 850)	face |= B_HEAVY_FACE;
	else if(weight >= 950)					face |= B_HEAVY_FACE;
#endif

	BFont* tempFont = new BFont();
	bool found = false;
	for(string_vector::iterator i = fonts.begin();
		i != fonts.end(); i++)
	{
		if (B_OK == tempFont->SetFamilyAndFace(i->c_str(),face))
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		// default to the Be plain font
		tempFont = new BFont(be_plain_font);
		if (weight >= 550)
		{
			tempFont = new BFont(be_bold_font);
		}
		tempFont->SetFace(face); // chooses closest
	}

	tempFont->SetSize(size);

	font_height hgt;
	tempFont->GetHeight(&hgt);
	fm->ascent = hgt.ascent;
	fm->descent = hgt.descent;
	fm->height = (int) (hgt.ascent + hgt.descent);
	fm->x_height = (int) hgt.leading;

	return (uint_ptr) tempFont;
}

void
LiteHtmlView::delete_font( uint_ptr hFont )
{
    // TODO: not implemented - do we need to do something here?
}

int
LiteHtmlView::text_width( const char* text,
	uint_ptr hFont )
{
	//std::cout << "text_width" << std::endl;
	BFont* fnt = (BFont*)hFont;
	int width = fnt->StringWidth(text);
	//std::cout << "    Width: " << +width << std::endl;
	return width;
}

void
LiteHtmlView::draw_text( uint_ptr hdc, const char* text,
	uint_ptr hFont, web_color color,
	const position& pos )
{
	//std::cout << "draw_text" << std::endl;
	if (!text) return;
	if (0 == strlen(text)) return;
	BFont* fnt = (BFont*)hFont;
	BRect bounds(Bounds());

	//FillRect(bounds,B_SOLID_LOW);

	BPoint leftTop = bounds.LeftTop();

	font_height fh;
	fnt->GetHeight(&fh);
	int baseline = fh.ascent + fh.descent;// + 10;
	int leftbase = 0; //10;
	MovePenTo(pos.left() + leftbase,pos.top() + baseline);//leftTop.x,leftTop.y);
	SetFont(fnt);
	rgb_color clr = ui_color(B_DOCUMENT_TEXT_COLOR);

	clr.red = color.red;
	clr.green = color.green;
	clr.blue = color.blue;
	clr.alpha = color.alpha;

	//std::cout << "    Final RGBA: " << +clr.red << "," << +clr.green << "," << +clr.blue << "," << +clr.alpha << std::endl;
	SetHighColor(clr);
	SetLowColor(ui_color(B_DOCUMENT_BACKGROUND_COLOR));
	BString mystr("");
	mystr << text;
	DrawString(mystr);
}

int
LiteHtmlView::pt_to_px( int pt ) const
{
	std::cout << "pt_to_px" << std::endl;
	return (int) ((double) pt * 1.3333333333);
}

int
LiteHtmlView::get_default_font_size() const
{
	return be_plain_font->Size();
}

const char*
LiteHtmlView::get_default_font_name() const
{
	font_family fam;
	::font_style style;
	be_plain_font->GetFamilyAndStyle(&fam, &style);
    return (new string(fam))->c_str();
}

void
LiteHtmlView::draw_list_marker( uint_ptr hdc,
	const list_marker& marker )
{
	if (!marker.image.empty())
	{
		std::cout << "    draw list image marker" << std::endl;
	}
}

void
LiteHtmlView::load_image( const char* src,
	const char* baseUrl, bool redraw_on_ready )
{
	std::cout << "load_image" << std::endl;

	BUrl absoluteUrl;
	make_url(src, baseUrl, absoluteUrl);
	std::cout << "   load_image from absolute URL " << absoluteUrl << std::endl;

    uint32 urlKey = absoluteUrl.UrlString().HashValue();
    std::cout << "   checking image cache with key " << urlKey << std::endl;

	if (m_images.find(urlKey) == m_images.end())
	{
        std::cout << "   image not yet in cache, fetching..." << std::endl;
        const char* htmlContent = FetchHttpContent(absoluteUrl);
        if (htmlContent == NULL || strlen(htmlContent) == 0)
        {
            std::cout << "    no valid image data received, aborting." << std::endl;
            return;
        }
        else
        {
			std::cout << "    loaded image from data from " << absoluteUrl << std::endl;
			BBitmap* img = BTranslationUtils::GetBitmap(htmlContent);
			m_images[urlKey] = img;
		}
	} else {
        std::cout << "   found image, done." << std::endl;
    }
}

void
LiteHtmlView::make_url(const char* relativeUrl, const char* baseUrl, BUrl& outUrl)
{
    if (baseUrl == NULL || (baseUrl[0] == '\0')) {
        baseUrl = m_base_url.c_str();
    }
    if (relativeUrl == NULL) relativeUrl = "";
    std::cout << "make_url: base url = " << baseUrl << ", relative path = " << relativeUrl << std::endl;

    // strip current dir ./ from relative url since we always use an absolute url in the end
    BString path(relativeUrl);
    if (path.StartsWith("./")) {
        path = path.RemoveFirst("./");
    } else {
        // check for absolute URL in path
        BUrl url(relativeUrl);
        if (url.IsValid()) {
            outUrl = url;
        }
    }
    if (! outUrl.IsValid()) {
        BString url(baseUrl);
        url.Append(path);
        outUrl = url.String();
    }
	std::cout << "output url is " << (outUrl.IsValid() ? "valid" : "invalid!")
              << ", outUrl = " << outUrl << std::endl;
}

void
LiteHtmlView::set_base_url(const char* base_url)
{
    if (m_base_url.empty()) {
        m_base_url = base_url;
        std::cout << "base url set to: " << m_base_url << std::endl;
    }
}


void
LiteHtmlView::get_image_size( const char* src, const char* baseurl, size& sz)
{
	std::cout << "get_image_size for image " << baseurl << std::endl;
	BUrl url;
	make_url(src, baseurl, url);

	const auto& miter(m_images.find(url.UrlString().HashValue()));
	if (m_images.end() != miter)
	{
		BBitmap* img = (BBitmap*)miter->second;
        if (img) {
            BRect size = img->Bounds();
            sz.width = size.Width();
            sz.height = size.Height();
            std::cout << "    width: " << +sz.width << ", height: " << +sz.height << std::endl;
        } else {
            std::cout << "    could not get image!" << std::endl;
        }
	}
}

void
LiteHtmlView::draw_image(uint_ptr hdc, const background_layer& layer, const std::string& url, const std::string& base_url)
{
	const auto& img = m_images.find(BString::HashValue(url.c_str()));

	if(img != m_images.end())
	{
		if(img->second)
		{
            auto boundingBox(layer.border_box);

            BPoint topLeft(boundingBox.left(), boundingBox.top());
            BPoint bottomRight(boundingBox.right(), boundingBox.bottom());

            // scale bitmap size to target rect
            BRect imageRect(img->second->Bounds());
            BRect targetRect(topLeft, bottomRight);

			DrawBitmap(img->second, imageRect, targetRect);
		}
	}
}

void LiteHtmlView::draw_solid_fill(uint_ptr hdc, const background_layer& layer, const web_color& color)
{
	if (color == web_color::transparent)
        return;

    auto boundingBox(layer.border_box);

    BPoint topLeft(boundingBox.left(), boundingBox.top());
    BPoint bottomRight(boundingBox.right(), boundingBox.bottom());

    auto hiCol = HighColor();
    SetHighColor(color.red, color.green, color.blue, color.alpha);

    FillRect(
        BRect(
            topLeft,
            bottomRight
        )
    );
    // reset to old color
    SetHighColor(hiCol);
}

void LiteHtmlView::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient)
{
    //FIXME - implement
    std::cout << "draw_linear_gradient: NOT YET IMPLEMENTED" << std::endl;
    return;
}

void LiteHtmlView::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient)
{
    //FIXME - implement
    std::cout << "draw_radial_gradient: NOT YET IMPLEMENTED" << std::endl;
    return;
}

void LiteHtmlView::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient)
{
    //FIXME - implement
    std::cout << "draw_conic_gradient: NOT YET IMPLEMENTED" << std::endl;
    return;
}

void
LiteHtmlView::draw_borders(uint_ptr hdc, const borders& borders, const position& draw_pos, bool root)
{
	std::cout << "draw_borders" << std::endl;
	int bdr_top		= 0;
	int bdr_bottom	= 0;
	int bdr_left	= 0;
	int bdr_right	= 0;

	if(borders.top.width != 0 && borders.top.style > border_style_hidden)
	{
		bdr_top = (int) borders.top.width;
	}
	if(borders.bottom.width != 0 && borders.bottom.style > border_style_hidden)
	{
		bdr_bottom = (int) borders.bottom.width;
	}
	if(borders.left.width != 0 && borders.left.style > border_style_hidden)
	{
		bdr_left = (int) borders.left.width;
	}
	if(borders.right.width != 0 && borders.right.style > border_style_hidden)
	{
		bdr_right = (int) borders.right.width;
	}

	if (bdr_bottom)
	{
		// draw rectangle for now - no check for radius
		StrokeRect(
			BRect(
				BPoint(draw_pos.left(), draw_pos.bottom()),
				BPoint(draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom)
			)
		);
	}
}

void
LiteHtmlView::transform_text(string& text, text_transform tt)
{
	std::cout << "transform_text: NOT YET IMPLEMENTED" << std::endl;
}

void
LiteHtmlView::set_clip( const position& pos, const border_radiuses& bdr_radius )
{
	std::cout << "set_clip: NOT YET IMPLEMENTED" << std::endl;
}

void
LiteHtmlView::del_clip()
{
	std::cout << "del_clip: NOT YET IMPLEMENTED" << std::endl;
}

std::shared_ptr<element>
LiteHtmlView::create_element(const char *tag_name,
							 const string_map &attributes,
							 const std::shared_ptr<document> &doc)
{
    // not implemented (also not in Cairo)
	return nullptr;
}

void
LiteHtmlView::get_media_features(media_features& media) const
{
	std::cout << "get_media_features" << std::endl;
	position client;
    get_client_rect(client);
	media.type			= media_type_screen;
	media.width			= client.width;
	media.height		= client.height;
	BRect bounds(Bounds());
	media.device_width	= bounds.Width();
	media.device_height	= bounds.Height();
    //FIXME
	media.color			= 8;
	media.monochrome	= 0;
	media.color_index	= 256;
	media.resolution	= 96;
}

void
LiteHtmlView::link(const std::shared_ptr<document> &ptr, const element::ptr& el)
{
    const char* href = el->get_attr("href", "");
	std::cout << "link [href = '" << href << "'] NOT YET IMPLEMENTED" << std::endl;
}

void
LiteHtmlView::set_caption(const char* caption)
{
	std::cout << "set_caption ['" << caption << "']: NOT YET IMPLEMENTED" << std::endl;
}

void
LiteHtmlView::get_client_rect(position& client) const
{
	//std::cout << "get_client_rect" << std::endl;
	BRect bounds(Bounds());
	BPoint leftTop = bounds.LeftTop();

	client.width = bounds.Width();
	client.height = bounds.Height();
	client.x = leftTop.x;
	client.y = leftTop.y;
}

void LiteHtmlView::on_mouse_event(const element::ptr& el, mouse_event event)
{
    std::cout << "on_mouse_event: NOT YET IMPLEMENTED" << std::endl;
}

void
LiteHtmlView::on_anchor_click(const char* base, const element::ptr& anchor)
{
	std::cout << "on_anchor_click: NOT YET IMPLEMENTED" << std::endl;
}

void
LiteHtmlView::set_cursor(const char* cursor)
{
	std::cout << "set_cursor: NOT YET IMPLEMENTED" << std::endl;
}

void
LiteHtmlView::import_css(string& text, const string& url, string& baseUrl)
{
	BUrl absoluteUrl;
	make_url(url.c_str(), baseUrl.c_str(), absoluteUrl);

	std::cout << "import_css from " << absoluteUrl.UrlString() << std::endl;
	text = FetchHttpContent(absoluteUrl);
}

void
LiteHtmlView::get_language(string& s1, string& s2) const
{
	std::cout << "get_language: NOT YET IMPLEMENTED" << std::endl;
}
