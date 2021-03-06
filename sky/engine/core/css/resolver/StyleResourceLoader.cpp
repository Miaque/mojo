/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "sky/engine/core/css/resolver/StyleResourceLoader.h"

#include "gen/sky/core/CSSPropertyNames.h"
#include "sky/engine/core/css/CSSImageValue.h"
#include "sky/engine/core/css/resolver/ElementStyleResources.h"
#include "sky/engine/core/fetch/ResourceFetcher.h"
#include "sky/engine/core/rendering/style/FillLayer.h"
#include "sky/engine/core/rendering/style/RenderStyle.h"
#include "sky/engine/core/rendering/style/StyleFetchedImage.h"
#include "sky/engine/core/rendering/style/StyleFetchedImageSet.h"
#include "sky/engine/core/rendering/style/StyleGeneratedImage.h"
#include "sky/engine/core/rendering/style/StylePendingImage.h"

namespace blink {

StyleResourceLoader::StyleResourceLoader(ResourceFetcher* fetcher)
    : m_fetcher(fetcher)
{
}

PassRefPtr<StyleImage> StyleResourceLoader::loadPendingImage(StylePendingImage* pendingImage, float deviceScaleFactor)
{
    if (CSSImageValue* imageValue = pendingImage->cssImageValue())
        return imageValue->cachedImage(m_fetcher, ResourceFetcher::defaultResourceOptions());

    if (CSSImageGeneratorValue* imageGeneratorValue
        = pendingImage->cssImageGeneratorValue()) {
        imageGeneratorValue->loadSubimages(m_fetcher);
        return StyleGeneratedImage::create(imageGeneratorValue);
    }

    if (CSSImageSetValue* imageSetValue = pendingImage->cssImageSetValue())
        return imageSetValue->cachedImageSet(m_fetcher, deviceScaleFactor, ResourceFetcher::defaultResourceOptions());

    return nullptr;
}

void StyleResourceLoader::loadPendingImages(RenderStyle* style, ElementStyleResources& elementStyleResources)
{
    if (elementStyleResources.pendingImageProperties().isEmpty())
        return;

    PendingImagePropertyMap::const_iterator::Keys end = elementStyleResources.pendingImageProperties().end().keys();
    for (PendingImagePropertyMap::const_iterator::Keys it = elementStyleResources.pendingImageProperties().begin().keys(); it != end; ++it) {
        CSSPropertyID currentProperty = *it;

        switch (currentProperty) {
        case CSSPropertyBackgroundImage: {
            for (FillLayer* backgroundLayer = &style->accessBackgroundLayers(); backgroundLayer; backgroundLayer = backgroundLayer->next()) {
                if (backgroundLayer->image() && backgroundLayer->image()->isPendingImage())
                    backgroundLayer->setImage(loadPendingImage(toStylePendingImage(backgroundLayer->image()), elementStyleResources.deviceScaleFactor()));
            }
            break;
        }
        case CSSPropertyBorderImageSource: {
            if (style->borderImageSource() && style->borderImageSource()->isPendingImage())
                style->setBorderImageSource(loadPendingImage(toStylePendingImage(style->borderImageSource()), elementStyleResources.deviceScaleFactor()));
            break;
        }
        default:
            ASSERT_NOT_REACHED();
        }
    }

    elementStyleResources.clearPendingImageProperties();
}

void StyleResourceLoader::loadPendingResources(RenderStyle* renderStyle, ElementStyleResources& elementStyleResources)
{
    // Start loading images referenced by this style.
    loadPendingImages(renderStyle, elementStyleResources);
}

}
