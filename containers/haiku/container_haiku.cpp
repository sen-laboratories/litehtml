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

#include <Application.h>
#include <Bitmap.h>
#include <Cursor.h>
#include <String.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <TranslationUtils.h>

#include <private/netservices2/HttpRequest.h>
#include <private/netservices2/HttpResult.h>
#include <private/netservices2/NetServicesDefs.h>

using namespace litehtml;
using namespace BPrivate::Network;

LiteHtmlView::LiteHtmlView(BRect frame, const char *name)
	: BView(frame, name, B_FOLLOW_ALL, B_WILL_DRAW),
	m_doc(NULL),
	m_images(),
	m_base_url(),
	m_url()
{
	SetDrawingMode(B_OP_OVER);
	SetFont(be_plain_font);
    SetMouseEventMask(B_MOUSE_MOVED);
    fHttpSession = new BHttpSession();
    // to get mouse/keyboard events
    MakeFocus();
}

LiteHtmlView::~LiteHtmlView()
{
    delete fHttpSession;
}

void
LiteHtmlView::RenderUrl(const char* fileOrHttpUrl, const char* masterStylesPath, const char* userStylesPath)
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
    RenderUrl(url, masterStylesPath, userStylesPath);
}

void
LiteHtmlView::RenderUrl(const BUrl& url, const char* masterStylesPath, const char* userStylesPath)
{
	std::cout << "RenderUrl from URL " << url << std::endl;

    const char* html = FetchHttpContent(url);
    if (html != NULL)
    {
        RenderHtml(html, masterStylesPath, userStylesPath);
    }
    else
    {
        std::cout << "RenderUrl aborted, no content received." << std::endl;
    }
}

void
LiteHtmlView::RenderHtml(const BString& htmlText, const char* masterStylesPath, const char* userStylesPath)
{
	std::cout << "RenderHTML" << std::endl;

	// todo: load optionally supplied styles from paths and pass string content ref here
	m_doc = document::createFromString(htmlText.String(), this/*, string(masterStylesPath), string(userStylesPath)*/);

	if (m_doc)
	{
		std::cout << "Successfully read html" << std::endl;
		// success
		// post-parse render operations, if required.
		// done by caller Invalidate();
	} else {
		std::cout << "Failed to read html" << std::endl;
        return;
	}

	// always fire the rendering complete message
	//std::cout << "Sending html rendered message: " << M_HTML_RENDERED << std::endl;
    //SendNotices(M_HTML_RENDERED,new BMessage(M_HTML_RENDERED));
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
        return FetchLocalContentFromFile(fileOrHttpUrl);
    } else {
        return FetchRemoteContentFromUrl(fileOrHttpUrl);
	}
}

const BString&
LiteHtmlView::FetchLocalContentFromFile(const BUrl& fileOrHttpUrl)
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
}

const BString&
LiteHtmlView::FetchRemoteContentFromUrl(const BUrl& fileOrHttpUrl)
{
    std::cout << "  fetching from URL " << fileOrHttpUrl.UrlString() << std::endl;
    BString baseUrl(fileOrHttpUrl.Protocol());
    baseUrl << "://" << fileOrHttpUrl.Host() << "/";
    set_base_url(baseUrl);

    try {
        BHttpRequest&& request = BHttpRequest(fileOrHttpUrl);
        BHttpResult result = fHttpSession->Execute(std::move(request));
        if (result.Status().code >= 200 && result.Status().code <= 400)
        {
            std::cout << "  Request successful, got "
                      << (! result.Body().text.has_value() ? "empty " : "") << "response "
                      << (  result.Body().text.has_value() ? "with body" : "")
                      << " and status " << result.Status().code << ": " << result.Status().text
                      << std::endl;

            BString *htmlBody = new BString(result.Body().text.value_or(""));
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

void
LiteHtmlView::Draw(BRect b)
{
	std::cout << "DRAW CALLED" << std::endl;

	BRect bounds(Bounds());
	FillRect(bounds, B_SOLID_LOW);

	// b is only part of the window, but we need to draw the whole lot
    // TODO: still see how we can optimize this, clipping is wrong, view too large

	if (NULL != m_doc) {
		BPoint leftTop = bounds.LeftTop();
		position clip(leftTop.x, leftTop.y, bounds.Width(), bounds.Height());
		m_doc->render(bounds.Width());
		m_doc->draw((uint_ptr) this, 0, 0, &clip);
	}

    std::cout << "DRAW FINISHED, sending HTML RENDERED notice." << std::endl;
	SendNotices(M_HTML_RENDERED,new BMessage(M_HTML_RENDERED));
}

void LiteHtmlView::MouseDown(BPoint where)
{
    BPoint absoluteLoc;
    uint32 buttons;
    GetMouse(&absoluteLoc, &buttons);

    if (! (buttons | B_PRIMARY_MOUSE_BUTTON) ) {
        std::cout << "BView::MouseDown right/middle click, skipping." << std::endl;
        return;
    }

    // hand over mouse event to LiteHtml so we get invoked on our on_xx later
    litehtml::position::vector redrawBoxes;
    BRect client = GetClientRect();

    bool redraw = m_doc->on_lbutton_down(where.x, where.y, client.left, client.top, redrawBoxes);

    if (redraw) {
        std::cout << "  redraw boxes..." << std::endl;
        for (auto rect : redrawBoxes) {
            BRect invalidateRect(rect.left(), rect.top(), rect.right(), rect.bottom());
            BView::Invalidate(invalidateRect);
        }
    }
}

void LiteHtmlView::MouseUp(BPoint where)
{
    BPoint absoluteLoc;
    uint32 buttons;
    GetMouse(&absoluteLoc, &buttons);

    if (! (buttons | B_PRIMARY_MOUSE_BUTTON) ) {
        std::cout << "BView::MouseUp right/middle click, skipping." << std::endl;
        return;
    }

    // hand over mouse event to LiteHtml so we get invoked on our on_xx later
    litehtml::position::vector redrawBoxes;
    BRect client = GetClientRect();

    bool redraw = m_doc->on_lbutton_up(where.x, where.y, client.left, client.top, redrawBoxes);

    if (redraw) {
        std::cout << "  redraw boxes..." << std::endl;
        for (auto rect : redrawBoxes) {
            BRect invalidateRect(rect.left(), rect.top(), rect.right(), rect.bottom());
            BView::Invalidate(invalidateRect);
        }
    }
}

// hand over mouse event to LiteHtml so we get invoked on our on_xx later
void LiteHtmlView::MouseMoved(BPoint where, uint32 code, const BMessage *dragMessage)
{
    BPoint client = ConvertToParent(where);
    litehtml::position::vector redrawBoxes;
    bool redraw;

    switch(code) {
        case B_ENTERED_VIEW:
        case B_INSIDE_VIEW:
            redraw = m_doc->on_mouse_over(where.x, where.y, client.x, client.y, redrawBoxes);
            break;
        case B_EXITED_VIEW:
        case B_OUTSIDE_VIEW:
            redraw = m_doc->on_mouse_leave(redrawBoxes);
            break;
        default:
            std::cout << "unsupported/unknown code " << code << ", skipping." << std::endl;
    }

    if (redraw) {
        std::cout << "  redraw boxes..." << std::endl;
        for (auto rect : redrawBoxes) {
            BRect invalidateRect(rect.left(), rect.top(), rect.right(), rect.bottom());
            BView::Invalidate(invalidateRect);
        }
    }
}

void
LiteHtmlView::GetPreferredSize(float* width,float* height)
{
	if (NULL == m_doc)
	{
		BRect bounds(Bounds());
		*width = bounds.Width();
		*height = bounds.Height();
	} else {
		*width = m_doc->width();
		*height = m_doc->height();
	}
}

uint_ptr
LiteHtmlView::create_font( const char* faceName, int size,
	int weight, litehtml::font_style italic, unsigned int decoration,
	font_metrics* fm )
{
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
	for(string_vector::iterator i = fonts.begin(); i != fonts.end(); i++)
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
        const char* imageData = FetchHttpContent(absoluteUrl);
        if (imageData == NULL || strlen(imageData) == 0)
        {
            std::cout << "    no valid image data received, aborting." << std::endl;
            return;
        }
        else
        {
			std::cout << "    loaded image from data from " << absoluteUrl << std::endl;
			BBitmap* img = BTranslationUtils::GetBitmap(imageData);
            m_images[urlKey] = img;
            // we always save above to avoid cache miss and another attempt to download the image
            if (img == NULL) {
                std::cout << "      could not handle image " << absoluteUrl << std::endl;
            }
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
    std::cout << "make_url: base url = " << baseUrl;

    // WIP: handle data: URLs, e.g. data:image/svg+xml,... (URL escaped data like %3Csvg for <sgv)
    // see https://gist.github.com/jennyknuth/222825e315d45a738ed9d6e04c7a88d0
    BString relativeOrDataUrl(relativeUrl);
    BString dataPrefix("data:");

    if (relativeOrDataUrl.StartsWith(dataPrefix)) {
        int32 dataPrefixLen = dataPrefix.Length();
        int32 separatorOffset = relativeOrDataUrl.FindFirst(',', dataPrefixLen);
        if (separatorOffset < 0) {
            std::cout << "   data URL with illegal/unsupported format separator: "
                      << relativeOrDataUrl.TruncateChars(32) << std::endl;
        }
        BString dataFormat;
        relativeOrDataUrl.CopyCharsInto(dataFormat, dataPrefixLen, separatorOffset - dataPrefixLen);
        std::cout << ", data URL detected, format = " << dataFormat << std::endl;
        // TODO: what to do? caller must handle (probably already before calling this function)
        outUrl = BUrl(baseUrl);
        return;
    } else {
        std::cout << ", relative path = " << relativeUrl << std::endl;
    }
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
LiteHtmlView::get_image_size(const char* src, const char* baseUrl, size& sz)
{
	BUrl url;
	make_url(src, baseUrl, url);

    uint32 imgKey = url.UrlString().HashValue();

    auto imgIter = m_images.find(imgKey);
    if (imgIter != m_images.end() && imgIter->second != NULL) {
        BBitmap* img = (BBitmap*)imgIter->second;
        if (img) {
            std::cout << "get_image_size for image " << url << ": ";
            BRect size = img->Bounds();
            sz.width = size.Width();
            sz.height = size.Height();
            std::cout << "  width = " << sz.width << ", height = " << sz.height << std::endl;
        } else {
            std::cout << "    could not get image size for image src " << url << std::endl;
            sz.width = 0;
            sz.height = 0;
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

const BRect& LiteHtmlView::GetClientRect()
{
    position client;
    get_client_rect(client);
    return *(new BRect(client.x, client.y, client.x + client.width, client.y + client.height));
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
	media.color       = 8; // same as Chrome/Firefox
	media.monochrome  = 0; // same as Chrome/Firefox
	media.color_index = 0; // same as Chrome/Firefox - was 256 for Haiku
	media.resolution  = 96; // same as Chrome/Firefox
}

void
LiteHtmlView::link(const std::shared_ptr<document> &ptr, const element::ptr& el)
{
    const char* href = el->get_attr("href", "");
	std::cout << "adding link [href = '" << href << "']" << std::endl;
}

void
LiteHtmlView::set_caption(const char* caption)
{
	m_caption = caption;
}

void
LiteHtmlView::get_client_rect(position& client) const
{
	BRect bounds(Bounds());
	BPoint leftTop = bounds.LeftTop();

	client.width = bounds.IntegerWidth();
	client.height = bounds.IntegerHeight();
	client.x = leftTop.x;
	client.y = leftTop.y;
}

void LiteHtmlView::on_mouse_event(const element::ptr& el, mouse_event event)
{
    std::cout << "on_mouse_event: event = mouse_" << (event == 0 ? "enter" : "leave") << std::endl;
}

void
LiteHtmlView::on_anchor_click(const char* url, const element::ptr& anchor)
{
    BUrl href;
    make_url(url, NULL, href);

	std::cout << "on_anchor_click: url = " << href.UrlString()<< std::endl;
    BPoint location;
    uint32 buttons;
    GetMouse(&location, &buttons);

    BMessage msg(M_ANCHOR_CLICKED);
    msg.AddUInt32("buttons", buttons);
    msg.AddPoint("where", location);
    msg.AddString("href", href.UrlString());
    // get anchor
    if (href.HasFragment()) {
        msg.AddString("fragment", href.Fragment());
        // get position of anchor target
        // see https://github.com/litehtml/litehtml/wiki/How-to-use-litehtml#processing-named-anchors
        BString elName("#");
        elName += href.Fragment();
        element::ptr el = m_doc->root()->select_one(elName.String());
        if (el) {
            litehtml::position pos = el->get_placement();
            msg.AddPoint("fragmentPos", BPoint(0, pos.y));
        }
    }
    msg.PrintToStream();

    SendNotices(M_ANCHOR_CLICKED, new BMessage(msg));
}

void
LiteHtmlView::set_cursor(const char* cursor)
{
	std::cout << "set_cursor: " << cursor << std::endl;
    // TODO: implement lookup map between CSS properties and BCursorIDs,
    //  see: https://developer.mozilla.org/en-US/docs/Web/CSS/cursor and
    //       https://www.haiku-os.org/docs/api/Cursor_8h.html#a4e11bd0710deda12dc6d363e424fda3b
    if (strncmp(cursor, "pointer", 7) == 0) {
        std::cout << "  -> link cursor." << std::endl;
        BCursor mouseCursor(B_CURSOR_ID_FOLLOW_LINK);
    } else  {
        std::cout << "  -> default cursor." << std::endl;
        BCursor mouseCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
    }
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
