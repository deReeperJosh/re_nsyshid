/*
* Code repurposed from re_nfpii
*/

#include "ConfigItemSelectInfinity.hpp"
#include "devices/Infinity.h"
#include "utils/DrawUtils.hpp"
#include "utils/input.h"
#include "utils/logger.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sstream>
#include <sys/stat.h>

#include <coreinit/title.h>
#include <padscore/kpad.h>
#include <vpad/input.h>

#include "dir_icon.inc"
#include "fav_icon.inc"

#define COLOR_BACKGROUND         Color(238, 238, 238, 255)
#define COLOR_TEXT               Color(51, 51, 51, 255)
#define COLOR_TEXT2              Color(72, 72, 72, 255)
#define COLOR_DISABLED           Color(255, 0, 0, 255)
#define COLOR_BORDER             Color(204, 204, 204, 255)
#define COLOR_BORDER_HIGHLIGHTED Color(0x3478e4FF)
#define COLOR_WHITE              Color(0xFFFFFFFF)
#define COLOR_BLACK              Color(0, 0, 0, 255)

// limit the maximum amount of entries in a dir, to avoid really slow Wii U FS
#define MAX_ENTRIES_PER_DIR      256
// maximum entries that fit on a page
#define MAX_ENTRIES_PER_PAGE     12
// maximum filename length that fits on config menu screen
#define MAX_AMIIBO_NAME_LEN      45
// maximum path len that fits in the top bar
#define MAX_DISPLAY_PATH_LEN     58

enum ListEntryType {
    LIST_ENTRY_TYPE_FILE,
    LIST_ENTRY_TYPE_DIR,
    LIST_ENTRY_TYPE_TOP,
};

struct ListEntry {
    std::string name;
    ListEntryType type;
    bool isFavorite;
};

static void ConfigItemSelectInfinity_onDelete(void *context);
static bool ConfigItemSelectInfinity_callCallback(void *context);

static std::vector<std::string> favorites;
static bool favoritesUpdated  = false;
static bool favoritesPerTitle = false;

std::vector<std::string> &ConfigItemSelectInfinity_GetFavorites(void) {
    return favorites;
}

void ConfigItemSelectInfinity_Init(std::string rootPath, bool favoritesPerTitle) {
    favorites.clear();
    favoritesUpdated = false;

    ::favoritesPerTitle      = favoritesPerTitle;
    std::string favoritesKey = "infinityfavorites";
    if (favoritesPerTitle) {
        uint64_t titleId = OSGetTitleID();
        char titleIdString[17];
        snprintf(titleIdString, sizeof(titleIdString), "%016llx", titleId);
        favoritesKey += titleIdString;
    }

    int32_t favoritesSize;
    if (WUPSStorageAPI_GetInt(nullptr, (favoritesKey + "Size").c_str(), &favoritesSize) != WUPS_STORAGE_ERROR_SUCCESS) {
        return;
    }

    char *favoritesString = new char[favoritesSize + 1];
    if (WUPSStorageAPI_GetString(nullptr, favoritesKey.c_str(), favoritesString, favoritesSize + 1, nullptr) != WUPS_STORAGE_ERROR_SUCCESS) {
        delete[] favoritesString;
        return;
    }

    std::stringstream stream(favoritesString);
    std::string fav;
    while (std::getline(stream, fav, ':')) {
        std::string path = rootPath + fav;
        struct stat sb;
        if (stat(path.c_str(), &sb) == 0 && (sb.st_mode & S_IFMT) == S_IFREG) {
            favorites.push_back(path);
        }
    }

    delete[] favoritesString;
}

static void saveFavorites(ConfigItemSelectInfinity *item) {
    if (!favoritesUpdated || favorites.size() == 0) {
        return;
    }

    // Store the favorites with ":" as the path separator
    // Strip the rootPath (/vol/external01/wiiu/re_nsyshid) prefix
    // In the end everything is stored as one string like "amiibo1.bin:folder1/amiibo2.bin:folder2/amiibo3.bin"
    std::string saveBuf;
    for (const auto &fav : favorites) {
        saveBuf += fav.substr(item->rootPath.size()) + ":";
    }

    // get rid of the last ':'
    saveBuf.resize(saveBuf.size() - 1);

    std::string favoritesKey = "infinityfavorites";
    if (favoritesPerTitle) {
        uint64_t titleId = OSGetTitleID();
        char titleIdString[17];
        snprintf(titleIdString, sizeof(titleIdString), "%016llx", titleId);
        favoritesKey += titleIdString;
    }

    WUPSStorageAPI_StoreString(nullptr, favoritesKey.c_str(), saveBuf.c_str());
    WUPSStorageAPI_StoreInt(nullptr, (favoritesKey + "Size").c_str(), saveBuf.size());

    favoritesUpdated = false;
}

static void enterSelectionMenu(ConfigItemSelectInfinity *item) {
    std::vector<ListEntry> entries;
    bool highlightSelected = true;
    bool openTidFolder     = true;

    // Init DrawUtils
    DrawUtils::initBuffers();
    if (!DrawUtils::initFont()) {
        return;
    }

    while (true) {
    refresh:;
        entries.clear();

        // Add top entry
        if (item->currentPath != item->rootPath) {
            entries.push_back(ListEntry{"..", LIST_ENTRY_TYPE_TOP});
        }

        // Populate list with entries from dir
        struct dirent *ent;
        DIR *dir = opendir(item->currentPath.c_str());
        if (dir) {
            for (int i = 0; i < MAX_ENTRIES_PER_DIR && (ent = readdir(dir)) != NULL; i++) {
                ListEntry entry;
                entry.name       = ent->d_name;
                entry.isFavorite = false;
                if (ent->d_type & DT_REG) {
                    entry.type = LIST_ENTRY_TYPE_FILE;

                    // check if this entry is in favorites
                    auto it = std::find(favorites.cbegin(), favorites.cend(), item->currentPath + entry.name);
                    if (it != favorites.cend()) {
                        entry.isFavorite = true;
                    }
                } else if (ent->d_type & DT_DIR) {
                    entry.type = LIST_ENTRY_TYPE_DIR;
                } else {
                    continue;
                }

                entries.push_back(entry);
            }

            closedir(dir);
        } else {
            DEBUG_FUNCTION_LINE("Cannot open '%s'", item->currentPath.c_str());
            return;
        }

        // check if there is a folder in the root which starts with the current TID
        if (openTidFolder && item->currentPath == item->rootPath) {
            uint64_t titleId = OSGetTitleID();
            char titleIdString[17];
            snprintf(titleIdString, sizeof(titleIdString), "%016llx", titleId);

            for (ListEntry &e : entries) {
                if (e.type == LIST_ENTRY_TYPE_DIR) {
                    if (strncasecmp(e.name.c_str(), titleIdString, 16) == 0) {
                        // open it if there is
                        item->currentPath += e.name + "/";
                        openTidFolder = false;
                        goto refresh;
                    }
                }
            }
        }
        openTidFolder = false;

        // sort files
        std::sort(entries.begin(), entries.end(),
                  [](ListEntry &a, ListEntry &b) {
                      // top dir entry is always at the top
                      if (a.type == LIST_ENTRY_TYPE_TOP) {
                          return true;
                      } else if (b.type == LIST_ENTRY_TYPE_TOP) {
                          return false;
                      }

                      // list dirs above files
                      if (a.type == LIST_ENTRY_TYPE_DIR && b.type == LIST_ENTRY_TYPE_FILE) {
                          return true;
                      } else if (a.type == LIST_ENTRY_TYPE_FILE && b.type == LIST_ENTRY_TYPE_DIR) {
                          return false;
                      }

                      // sort the rest alphabetically
                      return strcasecmp(a.name.c_str(), b.name.c_str()) <= 0;
                  });


        // Check if the current amiibo is part of the entries, and highlight if it is
        int32_t selected = -1;
        if (!item->selectedFigure.empty()) {
            for (size_t i = 0; i < entries.size(); ++i) {
                if (std::string(item->currentPath + entries[i].name).compare(item->selectedFigure) == 0) {
                    selected = (int) i;
                    break;
                }
            }
        }

        uint32_t currentIndex = (selected > 0 && highlightSelected) ? selected : 0;
        uint32_t start        = 0;
        uint32_t end          = std::min(entries.size(), (uint32_t) MAX_ENTRIES_PER_PAGE);

        // only highlight selected item once
        highlightSelected = false;

        bool redraw = true;

        VPADStatus vpad{};
        VPADReadError vpadError;
        KPADStatus kpad{};
        KPADError kpadError;

        while (true) {
            uint32_t buttonsTriggered = 0;

            VPADRead(VPAD_CHAN_0, &vpad, 1, &vpadError);
            if (vpadError == VPAD_READ_SUCCESS) {
                buttonsTriggered = vpad.trigger;
            }

            // read kpads and remap the buttons we need
            for (int i = 0; i < 4; i++) {
                if (KPADReadEx((KPADChan) i, &kpad, 1, &kpadError) > 0) {
                    if (kpadError != KPAD_ERROR_OK) {
                        continue;
                    }

                    if (kpad.extensionType == WPAD_EXT_CORE || kpad.extensionType == WPAD_EXT_NUNCHUK ||
                        kpad.extensionType == WPAD_EXT_MPLUS || kpad.extensionType == WPAD_EXT_MPLUS_NUNCHUK) {
                        buttonsTriggered |= remapWiiMoteButtons(kpad.trigger);
                    } else if (kpad.extensionType == WPAD_EXT_CLASSIC) {
                        buttonsTriggered |= remapClassicButtons(kpad.classic.trigger);
                    } else if (kpad.extensionType == WPAD_EXT_PRO_CONTROLLER) {
                        buttonsTriggered |= remapProButtons(kpad.pro.trigger);
                    }
                }
            }

            if (buttonsTriggered & VPAD_BUTTON_DOWN) {
                if (currentIndex < entries.size() - 1) {
                    currentIndex++;
                    redraw = true;
                }
            }

            if (buttonsTriggered & VPAD_BUTTON_UP) {
                if (currentIndex > 0) {
                    --currentIndex;
                    redraw = true;
                }
            }

            if (buttonsTriggered & VPAD_BUTTON_A) {
                ListEntry &currentEntry = entries[currentIndex];
                if (currentEntry.type == LIST_ENTRY_TYPE_FILE) {
                    selected             = currentIndex;
                    item->selectedFigure = item->currentPath + currentEntry.name;
                    redraw               = true;
                } else if (currentEntry.type == LIST_ENTRY_TYPE_DIR) {
                    item->currentPath += currentEntry.name + "/";
                    break;
                } else if (currentEntry.type == LIST_ENTRY_TYPE_TOP) {
                    // strip everything from last slash which isn't the trailing one
                    item->currentPath = item->currentPath.substr(0, item->currentPath.find_last_of("/", item->currentPath.length() - 2) + 1);
                    break;
                }
            }

            if (buttonsTriggered & VPAD_BUTTON_B) {
                // quit for the root path
                if (item->currentPath == item->rootPath) {
                    // calling this manually is bleh but that way the config entry gets updated immediately after returning
                    ConfigItemSelectInfinity_callCallback(item);
                    return;
                }

                // go one dir up for any other path
                // strip everything from last slash which isn't the trailing one
                item->currentPath = item->currentPath.substr(0, item->currentPath.find_last_of("/", item->currentPath.length() - 2) + 1);
                break;
            }

            if (buttonsTriggered & VPAD_BUTTON_X) {
                ListEntry &currentEntry = entries[currentIndex];
                if (currentEntry.type == LIST_ENTRY_TYPE_FILE) {
                    currentEntry.isFavorite = !currentEntry.isFavorite;

                    // add or remove from favorites
                    std::string absPath = item->currentPath + currentEntry.name;
                    if (currentEntry.isFavorite) {
                        favorites.push_back(absPath);
                    } else {
                        favorites.erase(std::find(favorites.cbegin(), favorites.cend(), absPath));
                    }

                    favoritesUpdated = true;
                    redraw           = true;
                }
            }

            if (buttonsTriggered & VPAD_BUTTON_HOME) {
                // calling this manually is bleh but that way the config entry gets updated immediately after returning
                ConfigItemSelectInfinity_callCallback(item);
                return;
            }

            // handle scrolling past the end
            if (currentIndex >= end) {
                end   = currentIndex + 1;
                start = end - MAX_ENTRIES_PER_PAGE;
            } else if (currentIndex < start) {
                start = currentIndex;
                end   = start + MAX_ENTRIES_PER_PAGE;
            }

            if (redraw) {
                DrawUtils::beginDraw();
                DrawUtils::clear(COLOR_BACKGROUND);

                // draw entries
                uint32_t index = 8 + 24 + 8 + 4;
                for (uint32_t i = start; i < end; i++) {
                    ListEntry &entry = entries[i];

                    DrawUtils::setFontColor(COLOR_TEXT);

                    if (i == currentIndex) {
                        DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 30, 3, COLOR_BORDER_HIGHLIGHTED);
                    } else {
                        DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 30, 2, COLOR_BORDER);
                    }

                    DrawUtils::setFontSize(16);
                    DrawUtils::print(28 + 16 * 2, index + 6 + 16, entry.name.c_str());

                    if (entry.type == LIST_ENTRY_TYPE_FILE) {
                        // draw selected icon
                        if (selected != -1 && i == (uint32_t) selected) {
                            DrawUtils::print(16 * 2, index + 6 + 16, "\u25c9");
                        } else {
                            DrawUtils::print(16 * 2, index + 6 + 16, "\u25cb");
                        }

                        if (entry.isFavorite) {
                            // draw favorite icon
                            for (uint32_t i = 0; i < sizeof(fav_icon) / sizeof(Color); ++i) {
                                DrawUtils::drawPixel(SCREEN_WIDTH - (16 * 3) + (i % 20), index + 4 + (i / 20), fav_icon[i]);
                            }
                        }
                    } else if (entry.type == LIST_ENTRY_TYPE_DIR) {
                        // draw directory icon
                        for (uint32_t i = 0; i < sizeof(dir_icon) / sizeof(Color); ++i) {
                            DrawUtils::drawPixel(16 * 2 + (i % 20), index + 4 + (i / 20), dir_icon[i]);
                        }
                    } else if (entry.type == LIST_ENTRY_TYPE_TOP) {
                        // draw up icon
                        DrawUtils::print(16 * 2, index + 6 + 16, "\ue092");
                    }

                    index += 29 + 4;
                }

                DrawUtils::setFontColor(COLOR_TEXT);

                // draw top bar
                DrawUtils::setFontSize(24);
                DrawUtils::print(16, 6 + 24, "re_nsyshid - Select Infinity Toy");
                DrawUtils::setFontSize(18);
                std::string path = item->currentPath.c_str();
                // remove root path
                path = path.substr(item->rootPath.size());
                // trim to make sure it fits
                if (path.length() > MAX_DISPLAY_PATH_LEN) {
                    path = "..." + path.substr(path.length() - MAX_DISPLAY_PATH_LEN);
                }
                DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, path.c_str(), true);
                DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

                // draw bottom bar
                DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
                DrawUtils::setFontSize(18);
                DrawUtils::print(16, SCREEN_HEIGHT - 10, "\ue07d Navigate ");
                DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue002 Favorite / \ue000 Select", true);

                // draw scroll indicators
                DrawUtils::setFontSize(24);
                if (end < entries.size()) {
                    DrawUtils::print(SCREEN_WIDTH / 2 + 12, SCREEN_HEIGHT - 32, "\ufe3e", true);
                }
                if (start > 0) {
                    DrawUtils::print(SCREEN_WIDTH / 2 + 12, 32 + 20, "\ufe3d", true);
                }

                // draw back button
                DrawUtils::setFontSize(18);
                const char *exitHint = "\ue001 Back";
                DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);

                DrawUtils::endDraw();
                redraw = false;
            }
        }
    }
}

static bool ConfigItemSelectInfinity_callCallback(void *context) {
    ConfigItemSelectInfinity *item = (ConfigItemSelectInfinity *) context;

    saveFavorites(item);

    if (item->callback && !item->selectedFigure.empty()) {
        FILE *figureFile = fopen(item->selectedFigure.c_str(), "r+b");
        if (!figureFile) {
            DEBUG_FUNCTION_LINE_ERR("Failed to open Infinity Toy file");
        } else {
            std::array<uint8_t, 0x10 * 0x14> fileData;
            const size_t ret_code = fread(fileData.data(), sizeof(fileData[0]), fileData.size(), figureFile);
            if (ret_code == fileData.size()) {
                int32_t figNum = g_infinitybase.LoadFigure(fileData, std::move(figureFile), item->slot);
                if (figNum == 0) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to load Infinity Toy file");
                } else {
                    item->figNum = figNum;
                }
            } else {
                DEBUG_FUNCTION_LINE_ERR("Infinity Toy file too small");
                fclose(figureFile);
            }
        }
        item->callback(item, item->selectedFigure.c_str(), item->slot);
        return true;
    }

    return false;
}

static void ConfigItemSelectInfinity_onInput(void *context, WUPSConfigSimplePadData input) {
    ConfigItemSelectInfinity *item = (ConfigItemSelectInfinity *) context;

    if ((input.buttons_d & WUPS_CONFIG_BUTTON_A) == WUPS_CONFIG_BUTTON_A) {
        enterSelectionMenu(item);
    } else if ((input.buttons_d & WUPS_CONFIG_BUTTON_X) == WUPS_CONFIG_BUTTON_X) {
        g_infinitybase.RemoveFigure(item->slot);
        item->selectedFigure.clear();
        item->figNum = 0;
    }
}

static bool ConfigItemSelectInfinity_isMovementAllowed(void *context) {
    return true;
}

static int32_t ConfigItemSelectInfinity_getCurrentValueDisplay(void *context, char *out_buf, int32_t out_size) {
    ConfigItemSelectInfinity *item = (ConfigItemSelectInfinity *) context;

    if (item->figNum == 0) {
        strncpy(out_buf, "None", out_size);
        return 0;
    }
    auto figure = g_infinitybase.FindFigure(item->figNum);

    strncpy(out_buf, figure.second.c_str(), out_size);
    return 0;
}

static void ConfigItemSelectInfinity_restoreDefault(void *context) {
    ConfigItemSelectInfinity *item = (ConfigItemSelectInfinity *) context;
    item->selectedFigure        = "";
    item->currentPath              = item->rootPath;
}

static void ConfigItemSelectInfinity_onSelected(void *context, bool isSelected) {
}

bool ConfigItemSelectInfinity_AddToCategory(WUPSConfigCategoryHandle cat, const char *configID, const char *displayName, uint8_t slot, const char *infinityFolder, const char *currentFigure, InfinityFigureSelectedCallback callback) {
    if (!displayName || !infinityFolder || !currentFigure) {
        return false;
    }

    ConfigItemSelectInfinity *item = new ConfigItemSelectInfinity;
    if (!item) {
        return false;
    }

    if (g_infinitybase.FindFigure(slot).second == "None") {
        item->selectedFigure = infinityFolder;
    } else {
        item->selectedFigure = currentFigure;
    }

    item->callback = callback;
    item->rootPath = infinityFolder;
    item->slot     = slot;

    // add a trailing "/" to work with the dir browse impl
    if (!item->rootPath.ends_with("/")) {
        item->rootPath += "/";
    }

    item->currentPath = item->rootPath;

    if (configID) {
        item->configID = strdup(configID);
    } else {
        item->configID = nullptr;
    }

    WUPSConfigAPIItemCallbacksV2 callbacks = {
            .getCurrentValueDisplay         = &ConfigItemSelectInfinity_getCurrentValueDisplay,
            .getCurrentValueSelectedDisplay = &ConfigItemSelectInfinity_getCurrentValueDisplay,
            .onSelected                     = &ConfigItemSelectInfinity_onSelected,
            .restoreDefault                 = &ConfigItemSelectInfinity_restoreDefault,
            .isMovementAllowed              = &ConfigItemSelectInfinity_isMovementAllowed,
            .onInput                        = &ConfigItemSelectInfinity_onInput,
            .onDelete                       = &ConfigItemSelectInfinity_onDelete,
            //.onCloseCallback                = &ConfigItemSelectInfinity_callCallback,
    };

    WUPSConfigAPIItemOptionsV2 options = {
            .displayName = displayName,
            .context     = item,
            .callbacks   = callbacks,
    };

    WUPSConfigAPIStatus err;
    if ((err = WUPSConfigAPI_Item_Create(options, &item->handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        delete item;
        return err;
    }

    if (WUPSConfigAPI_Category_AddItem(cat, item->handle) < 0) {
        return false;
    }

    return true;
}

static void ConfigItemSelectInfinity_onDelete(void *context) {
    ConfigItemSelectInfinity *item = (ConfigItemSelectInfinity *) context;

    free(item->configID);

    delete item;
}
