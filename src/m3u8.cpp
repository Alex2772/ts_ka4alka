//
// Created by Alex2772 on 4/27/2022.
//

#include "m3u8.h"
#include <AUI/Curl/ACurl.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Traits/strings.h>
#include <AUI/IO/AByteBufferInputStream.h>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Logging/ALogger.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>


/*
#EXTM3U
#EXT-X-TARGETDURATION:22
#EXT-X-ALLOW-CACHE:YES
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-KEY:METHOD=AES-128,URI="https://cs9-5v4.vkuseraudio.net/s/v1/ac/_fkQTXZkli0CKfybcKROPJzzcsxy1cC3b4FyG3Lr6nGIHU3zo7MOoROAdz6GlLavIft8Yz0_CK9bPgk4Uz5o7J0n8YSoFhaIbprndtLy9iTH-C-UXQIkoY1U5kAS6_XgYqlNg4qFD3PvxGOvcTypX1lS1oZ7OOIeBFLoMwiSh0FbTDg/key.pub"
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:1
#EXTINF:2.000,
seg-1-a1.ts
#EXT-X-KEY:METHOD=NONE
#EXTINF:4.000,
seg-2-a1.ts
#EXTINF:20.000,
seg-3-a1.ts
#EXT-X-KEY:METHOD=AES-128,URI="https://cs9-5v4.vkuseraudio.net/s/v1/ac/_fkQTXZkli0CKfybcKROPJzzcsxy1cC3b4FyG3Lr6nGIHU3zo7MOoROAdz6GlLavIft8Yz0_CK9bPgk4Uz5o7J0n8YSoFhaIbprndtLy9iTH-C-UXQIkoY1U5kAS6_XgYqlNg4qFD3PvxGOvcTypX1lS1oZ7OOIeBFLoMwiSh0FbTDg/key.pub"
#EXTINF:20.000,
seg-4-a1.ts
#EXT-X-KEY:METHOD=NONE
#EXTINF:20.000,
seg-5-a1.ts
#EXTINF:20.000,
seg-6-a1.ts
#EXT-X-KEY:METHOD=AES-128,URI="https://cs9-5v4.vkuseraudio.net/s/v1/ac/_fkQTXZkli0CKfybcKROPJzzcsxy1cC3b4FyG3Lr6nGIHU3zo7MOoROAdz6GlLavIft8Yz0_CK9bPgk4Uz5o7J0n8YSoFhaIbprndtLy9iTH-C-UXQIkoY1U5kAS6_XgYqlNg4qFD3PvxGOvcTypX1lS1oZ7OOIeBFLoMwiSh0FbTDg/key.pub"
#EXTINF:20.000,
seg-7-a1.ts
#EXT-X-KEY:METHOD=NONE
#EXTINF:20.000,
seg-8-a1.ts
#EXTINF:20.000,
seg-9-a1.ts
#EXT-X-KEY:METHOD=AES-128,URI="https://cs9-5v4.vkuseraudio.net/s/v1/ac/_fkQTXZkli0CKfybcKROPJzzcsxy1cC3b4FyG3Lr6nGIHU3zo7MOoROAdz6GlLavIft8Yz0_CK9bPgk4Uz5o7J0n8YSoFhaIbprndtLy9iTH-C-UXQIkoY1U5kAS6_XgYqlNg4qFD3PvxGOvcTypX1lS1oZ7OOIeBFLoMwiSh0FbTDg/key.pub"
#EXTINF:20.000,
seg-10-a1.ts
#EXT-X-KEY:METHOD=NONE
#EXTINF:22.128,
seg-11-a1.ts
#EXT-X-ENDLIST
*/

void m3u8::decode(const AString& url) {
    auto m3u8Buffer = ACurl::Builder(url).toByteBuffer();

    auto urlWithoutM3u8File = url.mid(0, url.rfind('/') + 1);

    ATokenizer tokenizer(_new<AByteBufferInputStream>(std::move(m3u8Buffer)));

    AFutureSet<AByteBuffer> futureSet;

    AES_KEY aesKey;
    bool usesDecryption = false;

    while (!tokenizer.isEof()) {
        auto line = tokenizer.readStringUntilUnescaped('\n').trimRight('\n').trim();
        if (!line.startsWith("#")) {
            // ts file, download it
            auto tsFileUrl = urlWithoutM3u8File + line;
            ALogger::info("m3u8") << "Downloading " << tsFileUrl;
            futureSet << asyncX [tsFileUrl = std::move(tsFileUrl), aesKey, usesDecryption] {
                auto rawBuffer = ACurl::Builder(tsFileUrl).toByteBuffer();
                if (usesDecryption) {
                    AByteBuffer decrypted;
                    decrypted.resize(rawBuffer.size());
                    AES_decrypt(reinterpret_cast<const unsigned char*>(rawBuffer.data()),
                                reinterpret_cast<unsigned char*>(decrypted.data()),
                                &aesKey);
                    unsigned char iv[16]={0};
                    std::memset(iv, 0, sizeof(iv));
                    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(rawBuffer.data()),
                                    reinterpret_cast<unsigned char*>(decrypted.data()),
                                    rawBuffer.size(),
                                    &aesKey,
                                    iv,
                                    AES_DECRYPT);

                    return decrypted;
                }

                return rawBuffer;
            };
        } else if (line.startsWith("#EXT-X-KEY")) {
            auto end = line.rfind('\"');
            if (end == AString::NPOS) {
                usesDecryption = false;
                continue;
            }
            auto begin = line.rfind('\"', end - 1);
            if (begin == AString::NPOS) {
                usesDecryption = false;
                continue;
            }
            auto keyUrl = line.mid(begin + 1, end - begin - 1);
            auto rawKey = ACurl::Builder(keyUrl).toByteBuffer();

            AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(rawKey.data()), rawKey.size() * 8, &aesKey);
            usesDecryption = true;
        }
    }

    // int index = 0;
    AFileOutputStream fos("output.ts");
    for (auto& f : futureSet) {
        //AFileOutputStream fos("output{}.ts"_format(index++));
        fos << *f;
    }
    ALogger::info("m3u8") << "Ready";
}