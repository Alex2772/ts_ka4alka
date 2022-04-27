#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AButton.h>

class MainWindow: public AWindow {
public:
    MainWindow();

private:
    _<ATextField> mUrl;
    AFuture<> mFuture;

    void onDownload();
};


