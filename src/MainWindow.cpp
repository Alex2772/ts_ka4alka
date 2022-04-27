//
// Created by Alex2772 on 4/27/2022.
//

#include "MainWindow.h"
#include "m3u8.h"
#include <AUI/Util/UIBuildingHelpers.h>

MainWindow::MainWindow():
    AWindow("ts_ka4alka")
{
    setContents(Centered {
        Vertical {
            _new<ALabel>("URL до m3u8 файла:"),
            mUrl = _new<ATextField>() let { it->setText("https://cs9-10v4.vkuseraudio.net/s/v1/ac/0d_WLlKv1lCFZfcn3rpzN42LsxPlClPKLB3a5234TTW11cOmsVDYXyjFIfRIAhP9x-9HoNTK9hkWyDGepRlTBX1N84Qf69rI33ufDVSjYQ737NIn0bXiRHFw4E4kCbqRVy5hsK8gDenZkULilY3XW3BbMwGWPv7o0tuDgCj5ES30zS0/index.m3u8"); },
            _new<AButton>("Качать").connect(&AButton::clicked, me::onDownload)
        },
    });
    pack();
    onDownload();
}

void MainWindow::onDownload() {
    auto url = mUrl->getText();

    mFuture = asyncX [url = std::move(url)] {
        m3u8::decode(url);
    };
}
