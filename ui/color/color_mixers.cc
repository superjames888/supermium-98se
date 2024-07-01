// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/color/color_mixers.h"

#include "base/files/file_util.h"
#include "base/path_service.h"
#include "chrome/common/chrome_paths.h"
#include "ui/base/ui_base_features.h"
#include "ui/color/color_mixer.h"
#include "ui/color/color_provider.h"
#include "ui/color/color_recipe.h"
#include "ui/color/core_default_color_mixer.h"
#include "ui/color/css_system_color_mixer.h"
#include "ui/color/fluent_ui_color_mixer.h"
#include "ui/color/material_ui_color_mixer.h"
#include "ui/color/native_color_mixers.h"
#include "ui/color/ref_color_mixer.h"
#include "ui/color/sys_color_mixer.h"
#include "ui/color/ui_color_mixer.h"
#include "ui/native_theme/native_theme_features.h"

namespace ui {
	
typedef struct DataItem {
	int datakey;
	std::vector<int> values;
}DataItem;
	
void AddUserDefinedMixer(ColorProvider* provider, const ColorProviderKey& key) {
	// This is the first implementation of the parser for user-defined customization values for Supermium components.
	ColorMixer& mixer = provider->AddMixer();
	
	int64_t file_size = 0;
	base::FilePath userdir;
	if(!base::PathService::Get(chrome::DIR_USER_DATA, &userdir))
		return; // Things are seriously wrong if the user data directory cannot be located.
	const base::FilePath userpath = userdir.Append(FILE_PATH_LITERAL("scs"));
	base::GetFileSize(userpath, &file_size);
	if(!file_size)
		return;
	std::vector<char> buf(file_size);
	base::ReadFile(userpath, reinterpret_cast<char*>(buf.data()), file_size);
	std::string bufstr = std::string(reinterpret_cast<char*>(buf.data()));
	
	std::string::size_type sectionstart = bufstr.find("colour");
	std::string::size_type sectionend = bufstr.find("endcolour");
	
	if(sectionstart == std::string::npos || sectionend == std::string::npos)
		return;
	
    sectionstart += std::string("colour").length();

    std::string sectionContent = bufstr.substr(sectionstart, sectionend - sectionstart);

    std::string::size_type pos = 0;
    while ((pos = sectionContent.find("{", pos)) != std::string::npos) {
        std::string::size_type endPos = sectionContent.find("}", pos);
        if (endPos == std::string::npos) {
            break;
        }

        std::string dataItemStr = sectionContent.substr(pos + 1, endPos - pos - 1);
        std::string::size_type equalPos = dataItemStr.find('=');

        if (equalPos != std::string::npos) {
            DataItem dataItem;
            dataItem.datakey = std::stoi(dataItemStr.substr(0, equalPos));
            std::string valuesStr = dataItemStr.substr(equalPos + 1);

            // Parse the values
            std::istringstream valuesStream(valuesStr);
            std::string value;
            while (std::getline(valuesStream, value, ',')) {
                dataItem.values.push_back(std::stoi(value));
            }

		    if(dataItem.datakey > 0 && dataItem.datakey < ui::kUiColorsEnd)
		        mixer[dataItem.datakey] = {SkColorSetRGB(dataItem.values.at(0), dataItem.values.at(1), dataItem.values.at(2))};
        }

        pos = endPos + 1;
    }
}

void AddColorMixers(ColorProvider* provider, const ColorProviderKey& key) {
  AddRefColorMixer(provider, key);
  // TODO(tluk): Determine the correct place to insert the sys color mixer.
  AddSysColorMixer(provider, key);
  AddCoreDefaultColorMixer(provider, key);
  AddNativeCoreColorMixer(provider, key);
  AddUiColorMixer(provider, key);
  if (features::IsChromeRefresh2023()) {
    // This must come after the UI and native UI mixers to ensure leaf node
    // colors are overridden with GM3 recipes when the refresh flag is enabled.
    AddMaterialUiColorMixer(provider, key);
  }
  if (IsFluentScrollbarEnabled()) {
    // This must come after the UI and before the native UI mixers to ensure
    // leaf node colors are overridden with the Fluent recipes but that high
    // contrast (specified via native UI on Windows) can override the Fluent
    // colors.
    AddFluentUiColorMixer(provider, key);
  }
  AddNativeUiColorMixer(provider, key);
  AddCssSystemColorMixer(provider, key);
  AddNativePostprocessingMixer(provider, key);
  AddUserDefinedMixer(provider, key);
}

}  // namespace ui
