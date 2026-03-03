#include <Geode/Geode.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>
using namespace geode::prelude;

// geode v4 never dies

#ifdef CCDICT_FOREACH
#undef CCDICT_FOREACH
#endif

#ifdef NO_DECLTYPE
#define HASH_ITER(hh, head, el, tmp)                                                      \
	for ((el) = (head), (*(char **)(&(tmp))) = (char *)((head) ? (head)->hh.next : NULL); \
	     el; (el) = (tmp), (*(char **)(&(tmp))) = (char *)((tmp) ? (tmp)->hh.next : NULL))
#else
#define HASH_ITER(hh, head, el, tmp)                                           \
	for ((el) = (head), (tmp) = DECLTYPE(el)((head) ? (head)->hh.next : NULL); \
	     el; (el) = (tmp), (tmp) = DECLTYPE(el)((tmp) ? (tmp)->hh.next : NULL))
#endif

#define CCDICT_FOREACH(__dict__, __el__)          \
	CCDictElement *pTmp##__dict__##__el__ = NULL; \
	if (__dict__)                                 \
	HASH_ITER(hh, (__dict__)->m_pElements, __el__, pTmp##__dict__##__el__)

#ifdef CCARRAY_FOREACH
#undef CCARRAY_FOREACH
#endif

#define CCARRAY_FOREACH(arr, elem)                    \
	for (size_t __i = 0; __i < (arr)->count(); ++__i) \
		if (auto elem = (arr)->objectAtIndex(__i))

namespace inlined {
/****************************************************************************
Copyright (c) Whoever owns https://github.com/altalk23/cocos2d-x-gd/blob/6bccfe7aecdbc32977395d50abcf385627b8f688/cocos2dx/sprite_nodes/CCSpriteFrameCache.cpp#L79
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009      Jason Booth
Copyright (c) 2009      Robert J Payne
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
void addSpriteFramesWithDictionary(CCDictionary *dictionary, std::string texturePath) {
	CCSpriteFrameCache *SpriteFrameCache = CCSpriteFrameCache::get();
	/*
	Supported Zwoptex Formats:

	ZWTCoordinatesFormatOptionXMLLegacy = 0, // Flash Version
	ZWTCoordinatesFormatOptionXML1_0 = 1, // Desktop Version 0.0 - 0.4b
	ZWTCoordinatesFormatOptionXML1_1 = 2, // Desktop Version 1.0.0 - 1.0.1
	ZWTCoordinatesFormatOptionXML1_2 = 3, // Desktop Version 1.0.2+
	*/

	CCDictionary *metadataDict = (CCDictionary *)dictionary->objectForKey("metadata");
	CCDictionary *framesDict = (CCDictionary *)dictionary->objectForKey("frames");
	int format = 0;

	// get the format
	if (metadataDict != NULL) {
		format = metadataDict->valueForKey("format")->intValue();
	}

	// check the format
	CCAssert(format >= 0 && format <= 3, "format is not supported for CCSpriteFrameCache addSpriteFramesWithDictionary:textureFilename:");
	CCTexture2D *pobTexture = NULL;
	CCDictElement *pElement = NULL;
	CCDICT_FOREACH(framesDict, pElement) {
		CCDictionary *frameDict = (CCDictionary *)pElement->getObject();
		std::string spriteFrameName = pElement->getStrKey();
		CCSpriteFrame *spriteFrame = (CCSpriteFrame *)SpriteFrameCache->m_pSpriteFrames->objectForKey(spriteFrameName);
		if (spriteFrame) {
			continue;
		}
		if (pobTexture == NULL) {
			pobTexture = CCTextureCache::sharedTextureCache()->addImage(texturePath.c_str(), false);
		};

		if (format == 0) {
			float x = frameDict->valueForKey("x")->floatValue();
			float y = frameDict->valueForKey("y")->floatValue();
			float w = frameDict->valueForKey("width")->floatValue();
			float h = frameDict->valueForKey("height")->floatValue();
			float ox = frameDict->valueForKey("offsetX")->floatValue();
			float oy = frameDict->valueForKey("offsetY")->floatValue();
			int ow = frameDict->valueForKey("originalWidth")->intValue();
			int oh = frameDict->valueForKey("originalHeight")->intValue();
			// check ow/oh
			if (!ow || !oh) {
				CCLOGWARN("cocos2d: WARNING: originalWidth/Height not found on the CCSpriteFrame. AnchorPoint won't work as expected. Regenrate the .plist");
			}
			// abs ow/oh
			ow = abs(ow);
			oh = abs(oh);
			// create frame
			spriteFrame = new CCSpriteFrame();
			spriteFrame->initWithTexture(pobTexture,
			                             CCRectMake(x, y, w, h),
			                             false,
			                             CCPointMake(ox, oy),
			                             CCSizeMake((float)ow, (float)oh));
		} else if (format == 1 || format == 2) {
			CCRect frame = CCRectFromString(frameDict->valueForKey("frame")->getCString());
			bool rotated = false;

			// rotation
			if (format == 2) {
				rotated = frameDict->valueForKey("rotated")->boolValue();
			}

			CCPoint offset = CCPointFromString(frameDict->valueForKey("offset")->getCString());
			CCSize sourceSize = CCSizeFromString(frameDict->valueForKey("sourceSize")->getCString());

			// create frame
			spriteFrame = new CCSpriteFrame();
			spriteFrame->initWithTexture(pobTexture,
			                             frame,
			                             rotated,
			                             offset,
			                             sourceSize);
		} else if (format == 3) {
			// get values
			CCSize spriteSize = CCSizeFromString(frameDict->valueForKey("spriteSize")->getCString());
			CCPoint spriteOffset = CCPointFromString(frameDict->valueForKey("spriteOffset")->getCString());
			CCSize spriteSourceSize = CCSizeFromString(frameDict->valueForKey("spriteSourceSize")->getCString());
			CCRect textureRect = CCRectFromString(frameDict->valueForKey("textureRect")->getCString());
			bool textureRotated = frameDict->valueForKey("textureRotated")->boolValue();

			// get aliases
			CCArray *aliases = (CCArray *)(frameDict->objectForKey("aliases"));
			CCString *frameKey = new CCString(spriteFrameName);

			CCObject *pObj = NULL;
			CCARRAY_FOREACH(aliases, pObj) {
				std::string oneAlias = ((CCString *)pObj)->getCString();

				SpriteFrameCache->m_pSpriteFramesAliases->setObject(frameKey, oneAlias.c_str());
			}
			frameKey->release();
			// create frame
			spriteFrame = new CCSpriteFrame();
			spriteFrame->initWithTexture(pobTexture,
			                             CCRectMake(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
			                             textureRotated,
			                             spriteOffset,
			                             spriteSourceSize);
		}

		// add sprite frame
		SpriteFrameCache->m_pSpriteFrames->setObject(spriteFrame, spriteFrameName);
		spriteFrame->release();
	}
};
}; // namespace inlined

// My cool function for Quality //
std::string getPlistForQuality(const char *pszPlist, cocos2d::TextureQuality quality) {
	std::string plistStr(pszPlist);

	size_t dotPos = plistStr.find_last_of(".");
	if (dotPos == std::string::npos) {
		dotPos = plistStr.length();
	}

	std::string baseName = plistStr.substr(0, dotPos);

	bool hasHD = baseName.size() >= 3 && baseName.compare(baseName.size() - 3, 3, "-hd") == 0;
	bool hasUHD = baseName.size() >= 4 && baseName.compare(baseName.size() - 4, 4, "-uhd") == 0;

	switch (quality) {
	case cocos2d::TextureQuality::kTextureQualityHigh: {
		if (!hasUHD) {
			plistStr.insert(dotPos, "-uhd");
		}
		break;
	}
	case cocos2d::TextureQuality::kTextureQualityMedium: {
		if (!hasHD && !hasUHD) {
			plistStr.insert(dotPos, "-hd");
		}
		break;
	}
	default: {
		break;
	}
	}

	return plistStr;
}

// My hook //
class $modify(RewritenSpriteFrames, CCSpriteFrameCache) {

	void addSpriteFramesWithFile(const char *pszPlist) {

		CCFileUtils *fileUtils = CCFileUtils::sharedFileUtils();
		if (fileUtils->isAbsolutePath(pszPlist)) {
			return CCSpriteFrameCache::addSpriteFramesWithFile(pszPlist);
		}
		std::string pszPlistStr = getPlistForQuality(pszPlist, CCDirector::get()->getLoadedTextureQuality());
		pszPlist = pszPlistStr.c_str();
		if (m_pLoadedFileNames->find(pszPlist) == m_pLoadedFileNames->end()) {
			m_pLoadedFileNames->insert(pszPlist);
			auto searchPaths = fileUtils->getSearchPaths();
			for (auto it = searchPaths.begin(); it != searchPaths.end(); ++it) {
				std::string fullPlist = ([](const std::string &d, const char *f) {
					std::string dir = d, file = f;
					std::replace(dir.begin(), dir.end(), '\\', '/');
					std::replace(file.begin(), file.end(), '\\', '/');
					while (!dir.empty() && dir.back() == '/')
						dir.pop_back();
					while (!file.empty() && file.front() == '/')
						file.erase(0, 1);
					return dir.empty() ? file : dir + '/' + file;
				})(*it, pszPlist);
				if (!fileUtils->isFileExist(fullPlist)) {
					// geode::log::debug("unable to load texture {}", fullPlist);
					continue;
				} /*else {
				    log::debug("loaded {}", fullPlist);
				};*/
				CCDictionary *dict = CCDictionary::createWithContentsOfFileThreadSafe(fullPlist.c_str());

				std::string texturePath("");

				cocos2d::CCDictionary *metadataDict = (CCDictionary *)dict->objectForKey("metadata");
				if (metadataDict) {
					texturePath = ([](const std::string &d, const char *f) {
						std::string dir = d, file = f;
						std::replace(dir.begin(), dir.end(), '\\', '/');
						std::replace(file.begin(), file.end(), '\\', '/');
						while (!dir.empty() && dir.back() == '/')
							dir.pop_back();
						while (!file.empty() && file.front() == '/')
							file.erase(0, 1);
						return dir.empty() ? file : dir + '/' + file;
					})(*it, metadataDict->valueForKey("textureFileName")->getCString());
				}

				if (texturePath.empty()) {
					texturePath = pszPlist;
					size_t startPos = texturePath.find_last_of(".");
					texturePath = texturePath.erase(startPos);
					texturePath = texturePath.append(".png");
				}
				inlined::addSpriteFramesWithDictionary(dict, texturePath);
				dict->release();
			}
		}
	}
};