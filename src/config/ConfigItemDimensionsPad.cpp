/*
* Code repurposed from re_nfpii
*/

#include "ConfigItemDimensionsPad.hpp"
#include "devices/Dimensions.h"
#include "utils/DrawUtils.hpp"
#include "utils/input.h"
#include "utils/logger.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <optional>
#include <sstream>
#include <sys/stat.h>

#include <coreinit/title.h>
#include <padscore/kpad.h>
#include <vpad/input.h>

#include "dir_icon.inc"
#include "fav_icon.inc"

#define COLOR_BACKGROUND            Color(238, 238, 238, 255)
#define COLOR_TEXT                  Color(51, 51, 51, 255)
#define COLOR_TEXT2                 Color(72, 72, 72, 255)
#define COLOR_DISABLED              Color(255, 0, 0, 255)
#define COLOR_BORDER                Color(204, 204, 204, 255)
#define COLOR_BORDER_MOVE           Color(0, 0, 0, 136)
#define COLOR_BORDER_HIGHLIGHTED    Color(0x3478e4FF)
#define COLOR_BORDER_MOVE_HIGHLIGHT Color(0x3478e488)
#define COLOR_WHITE                 Color(0xFFFFFFFF)
#define COLOR_BLACK                 Color(0, 0, 0, 255)

// limit the maximum amount of entries in a dir, to avoid really slow Wii U FS
#define MAX_ENTRIES_PER_DIR         256
// maximum entries that fit on a page
#define MAX_ENTRIES_PER_PAGE        12
// maximum filename length that fits on config menu screen
#define MAX_AMIIBO_NAME_LEN         45
// maximum path len that fits in the top bar
#define MAX_DISPLAY_PATH_LEN        58

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

static void ConfigItemDimensionsPad_onDelete(void *context);
static bool ConfigItemDimensionsPad_callCallback(void *context, uint8_t index);

static std::vector<std::string> favorites;
static bool favoritesUpdated  = false;
static bool favoritesPerTitle = false;

std::vector<std::string> &ConfigItemDimensionsPad_GetFavorites(void) {
    return favorites;
}

void ConfigItemDimensionsPad_Init(std::string rootPath, bool favoritesPerTitle) {
    favorites.clear();
    favoritesUpdated = false;

    ::favoritesPerTitle      = favoritesPerTitle;
    std::string favoritesKey = "dimensionFavorites";
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

static uint8_t derivePadFromIndex(uint8_t index) {
    switch (index) {
        case 0:
        case 3:
        case 4:
            return 1;

        case 2:
        case 5:
        case 6:
            return 3;

        case 1:
            return 2;


        default:
            return 0;
    }
}

static void saveFavorites(ConfigItemDimensionsPad *item) {
    if (!favoritesUpdated || favorites.size() == 0) {
        return;
    }

    // Store the favorites with ":" as the path separator
    // Strip the rootPath (/vol/external01/wiiu/re_nfpii) prefix
    // In the end everything is stored as one string like "amiibo1.bin:folder1/amiibo2.bin:folder2/amiibo3.bin"
    std::string saveBuf;
    for (const auto &fav : favorites) {
        saveBuf += fav.substr(item->rootPath.size()) + ":";
    }

    // get rid of the last ':'
    saveBuf.resize(saveBuf.size() - 1);

    std::string favoritesKey = "dimensionFavorites";
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

static void enterSelectionMenu(ConfigItemDimensionsPad *item, uint8_t index) {
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
        if (item->figureFiles[index]) {
            for (size_t i = 0; i < entries.size(); ++i) {
                if (std::string(item->currentPath + entries[i].name).compare(item->figureFiles[index].value()) == 0) {
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
                    selected                 = currentIndex;
                    item->figureFiles[index] = item->currentPath + currentEntry.name;
                    redraw                   = true;
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
                    ConfigItemDimensionsPad_callCallback(item, index);
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
                ConfigItemDimensionsPad_callCallback(item, index);
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
                DrawUtils::print(16, 6 + 24, "re_nsyshid - Dimensions Toypad");
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

static void enterToypadMenu(ConfigItemDimensionsPad *item) {
    // Init DrawUtils
    DrawUtils::initBuffers();
    if (!DrawUtils::initFont()) {
        return;
    }

    bool redraw = true;

    VPADStatus vpad{};
    VPADReadError vpadError;
    KPADStatus kpad{};
    KPADError kpadError;

    uint8_t currentIndex = 0;
    item->figureNumbers  = g_dimensionstoypad.GetCurrentFigures();

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

        if (buttonsTriggered & VPAD_BUTTON_B) {
            if (item->moveIndex) {
                item->moveIndex = std::nullopt;
                redraw          = true;
            } else {
                return;
            }
        }

        if (buttonsTriggered & VPAD_BUTTON_X) {
            if (!item->moveIndex) {
                uint8_t pad = derivePadFromIndex(currentIndex);
                if (pad > 0) {
                    g_dimensionstoypad.RemoveFigure(pad, currentIndex, true);
                    item->figureFiles[currentIndex]   = std::nullopt;
                    item->figureNumbers[currentIndex] = std::nullopt;
                    redraw                            = true;
                }
            }
        }

        if (buttonsTriggered & VPAD_BUTTON_A) {
            if (item->moveIndex) {
                if (currentIndex == item->moveIndex.value()) {
                    g_dimensionstoypad.CancelRemove(currentIndex);
                    item->moveIndex = std::nullopt;
                } else {
                    g_dimensionstoypad.MoveFigure(derivePadFromIndex(currentIndex), currentIndex, derivePadFromIndex(item->moveIndex.value()), item->moveIndex.value());
                    item->figureNumbers[currentIndex]            = item->figureNumbers[item->moveIndex.value()];
                    item->figureNumbers[item->moveIndex.value()] = std::nullopt;
                    item->figureFiles[currentIndex]              = item->figureFiles[item->moveIndex.value()];
                    item->figureFiles[item->moveIndex.value()]   = std::nullopt;
                    item->moveIndex                              = std::nullopt;
                }
            } else {
                if (item->figureNumbers[currentIndex]) {
                    item->moveIndex = currentIndex;
                    g_dimensionstoypad.TempRemove(currentIndex);
                } else {
                    enterSelectionMenu(item, currentIndex);
                }
            }
            redraw = true;
        }

        if (buttonsTriggered & VPAD_BUTTON_UP) {
            if (currentIndex == 3) {
                currentIndex = 0;
                redraw       = true;
            } else if (currentIndex == 4 || currentIndex == 5) {
                currentIndex = 1;
                redraw       = true;
            } else if (currentIndex == 6) {
                currentIndex = 2;
                redraw       = true;
            }
        }

        if (buttonsTriggered & VPAD_BUTTON_DOWN) {
            if (currentIndex == 0) {
                currentIndex = 3;
                redraw       = true;
            } else if (currentIndex == 1) {
                currentIndex = 4;
                redraw       = true;
            } else if (currentIndex == 2) {
                currentIndex = 6;
                redraw       = true;
            }
        }

        if (buttonsTriggered & VPAD_BUTTON_LEFT) {
            if (currentIndex == 1) {
                currentIndex = 0;
                redraw       = true;
            } else if (currentIndex == 2) {
                currentIndex = 1;
                redraw       = true;
            } else if (currentIndex == 4) {
                currentIndex = 3;
                redraw       = true;
            } else if (currentIndex == 5) {
                currentIndex = 4;
                redraw       = true;
            } else if (currentIndex == 6) {
                currentIndex = 5;
                redraw       = true;
            }
        }

        if (buttonsTriggered & VPAD_BUTTON_RIGHT) {
            if (currentIndex == 0) {
                currentIndex = 1;
                redraw       = true;
            } else if (currentIndex == 1) {
                currentIndex = 2;
                redraw       = true;
            } else if (currentIndex == 3) {
                currentIndex = 4;
                redraw       = true;
            } else if (currentIndex == 4) {
                currentIndex = 5;
                redraw       = true;
            } else if (currentIndex == 5) {
                currentIndex = 6;
                redraw       = true;
            }
        }

        if (redraw) {
            DrawUtils::beginDraw();
            DrawUtils::clear(COLOR_BACKGROUND);

            DrawUtils::setFontColor(COLOR_TEXT);

            // draw top bar
            DrawUtils::setFontSize(24);
            DrawUtils::print(16, 6 + 24, "re_nsyshid - Dimensions Toypad");
            DrawUtils::setFontSize(18);
            DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, "Toypad View", true);
            DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

            // Draw Dimension Slots
            if (currentIndex == 0) {
                if (item->moveIndex && item->moveIndex.value() == currentIndex) {
                    DrawUtils::drawRect(32, 80, 150, 120, 3, COLOR_BORDER_MOVE_HIGHLIGHT);
                } else {
                    DrawUtils::drawRect(32, 80, 150, 120, 3, COLOR_BORDER_HIGHLIGHTED);
                }
            } else {
                if (item->moveIndex && item->moveIndex.value() == 0) {
                    DrawUtils::drawRect(32, 80, 150, 120, 2, COLOR_BORDER_MOVE);
                } else {
                    DrawUtils::drawRect(32, 80, 150, 120, 2, COLOR_BLACK);
                }
            }
            if (currentIndex == 1) {
                if (item->moveIndex && item->moveIndex.value() == currentIndex) {
                    DrawUtils::drawRect(352, 80, 150, 120, 3, COLOR_BORDER_MOVE_HIGHLIGHT);
                } else {
                    DrawUtils::drawRect(352, 80, 150, 120, 3, COLOR_BORDER_HIGHLIGHTED);
                }
            } else {
                if (item->moveIndex && item->moveIndex.value() == 1) {
                    DrawUtils::drawRect(352, 80, 150, 120, 2, COLOR_BORDER_MOVE);
                } else {
                    DrawUtils::drawRect(352, 80, 150, 120, 2, COLOR_BLACK);
                }
            }
            if (currentIndex == 2) {
                if (item->moveIndex && item->moveIndex.value() == currentIndex) {
                    DrawUtils::drawRect(672, 80, 150, 120, 3, COLOR_BORDER_MOVE_HIGHLIGHT);
                } else {
                    DrawUtils::drawRect(672, 80, 150, 120, 3, COLOR_BORDER_HIGHLIGHTED);
                }
            } else {
                if (item->moveIndex && item->moveIndex.value() == 2) {
                    DrawUtils::drawRect(672, 80, 150, 120, 2, COLOR_BORDER_MOVE);
                } else {
                    DrawUtils::drawRect(672, 80, 150, 120, 2, COLOR_BLACK);
                }
            }
            if (currentIndex == 3) {
                if (item->moveIndex && item->moveIndex.value() == currentIndex) {
                    DrawUtils::drawRect(32, 280, 150, 120, 3, COLOR_BORDER_MOVE_HIGHLIGHT);
                } else {
                    DrawUtils::drawRect(32, 280, 150, 120, 3, COLOR_BORDER_HIGHLIGHTED);
                }
            } else {
                if (item->moveIndex && item->moveIndex.value() == 3) {
                    DrawUtils::drawRect(32, 280, 150, 120, 2, COLOR_BORDER_MOVE);
                } else {
                    DrawUtils::drawRect(32, 280, 150, 120, 2, COLOR_BLACK);
                }
            }
            if (currentIndex == 4) {
                if (item->moveIndex && item->moveIndex.value() == currentIndex) {
                    DrawUtils::drawRect(192, 280, 150, 120, 3, COLOR_BORDER_MOVE_HIGHLIGHT);
                } else {
                    DrawUtils::drawRect(192, 280, 150, 120, 3, COLOR_BORDER_HIGHLIGHTED);
                }
            } else {
                if (item->moveIndex && item->moveIndex.value() == 4) {
                    DrawUtils::drawRect(192, 280, 150, 120, 2, COLOR_BORDER_MOVE);
                } else {
                    DrawUtils::drawRect(192, 280, 150, 120, 2, COLOR_BLACK);
                }
            }
            if (currentIndex == 5) {
                if (item->moveIndex && item->moveIndex.value() == currentIndex) {
                    DrawUtils::drawRect(512, 280, 150, 120, 3, COLOR_BORDER_MOVE_HIGHLIGHT);
                } else {
                    DrawUtils::drawRect(512, 280, 150, 120, 3, COLOR_BORDER_HIGHLIGHTED);
                }
            } else {
                if (item->moveIndex && item->moveIndex.value() == 5) {
                    DrawUtils::drawRect(512, 280, 150, 120, 2, COLOR_BORDER_MOVE);
                } else {
                    DrawUtils::drawRect(512, 280, 150, 120, 2, COLOR_BLACK);
                }
            }
            if (currentIndex == 6) {
                if (item->moveIndex && item->moveIndex.value() == currentIndex) {
                    DrawUtils::drawRect(672, 280, 150, 120, 3, COLOR_BORDER_MOVE_HIGHLIGHT);
                } else {
                    DrawUtils::drawRect(672, 280, 150, 120, 3, COLOR_BORDER_HIGHLIGHTED);
                }
            } else {
                if (item->moveIndex && item->moveIndex.value() == 6) {
                    DrawUtils::drawRect(672, 280, 150, 120, 2, COLOR_BORDER_MOVE);
                } else {
                    DrawUtils::drawRect(672, 280, 150, 120, 2, COLOR_BLACK);
                }
            }

            // Draw currently selected figures
            DrawUtils::setFontSize(16);

            if (item->figureNumbers[0]) {
                std::string figureName = g_dimensionstoypad.FindFigure(item->figureNumbers[0].value());
                DrawUtils::print(32 + (75 - (DrawUtils::getTextWidth(figureName.c_str()) / 2)), 140, figureName.c_str());
            } else {
                DrawUtils::print(32 + (75 - (DrawUtils::getTextWidth("None") / 2)), 140, "None");
            }
            if (item->figureNumbers[1]) {
                std::string figureName = g_dimensionstoypad.FindFigure(item->figureNumbers[1].value());
                DrawUtils::print(352 + (75 - (DrawUtils::getTextWidth(figureName.c_str()) / 2)), 140, figureName.c_str());
            } else {
                DrawUtils::print(352 + (75 - (DrawUtils::getTextWidth("None") / 2)), 140, "None");
            }
            if (item->figureNumbers[2]) {
                std::string figureName = g_dimensionstoypad.FindFigure(item->figureNumbers[2].value());
                DrawUtils::print(672 + (75 - (DrawUtils::getTextWidth(figureName.c_str()) / 2)), 140, figureName.c_str());
            } else {
                DrawUtils::print(672 + (75 - (DrawUtils::getTextWidth("None") / 2)), 140, "None");
            }
            if (item->figureNumbers[3]) {
                std::string figureName = g_dimensionstoypad.FindFigure(item->figureNumbers[3].value());
                DrawUtils::print(32 + (75 - (DrawUtils::getTextWidth(figureName.c_str()) / 2)), 340, figureName.c_str());
            } else {
                DrawUtils::print(32 + (75 - (DrawUtils::getTextWidth("None") / 2)), 340, "None");
            }
            if (item->figureNumbers[4]) {
                std::string figureName = g_dimensionstoypad.FindFigure(item->figureNumbers[4].value());
                DrawUtils::print(192 + (75 - (DrawUtils::getTextWidth(figureName.c_str()) / 2)), 340, figureName.c_str());
            } else {
                DrawUtils::print(192 + (75 - (DrawUtils::getTextWidth("None") / 2)), 340, "None");
            }
            if (item->figureNumbers[5]) {
                std::string figureName = g_dimensionstoypad.FindFigure(item->figureNumbers[5].value());
                DrawUtils::print(512 + (75 - (DrawUtils::getTextWidth(figureName.c_str()) / 2)), 340, figureName.c_str());
            } else {
                DrawUtils::print(512 + (75 - (DrawUtils::getTextWidth("None") / 2)), 340, "None");
            }
            if (item->figureNumbers[6]) {
                std::string figureName = g_dimensionstoypad.FindFigure(item->figureNumbers[6].value());
                DrawUtils::print(672 + (75 - (DrawUtils::getTextWidth(figureName.c_str()) / 2)), 340, figureName.c_str());
            } else {
                DrawUtils::print(672 + (75 - (DrawUtils::getTextWidth("None") / 2)), 340, "None");
            }

            // draw bottom bar
            DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
            DrawUtils::setFontSize(18);
            DrawUtils::print(16, SCREEN_HEIGHT - 10, "\ue07d / \ue07e Navigate ");
            if (item->moveIndex) {
                DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue000 Move Here", true);
            } else {
                if (item->figureFiles[currentIndex]) {
                    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue002 Remove / \ue000 Pick Up", true);
                } else {
                    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue002 Remove / \ue000 Select", true);
                }
            }

            // draw back button
            DrawUtils::setFontSize(18);
            if (item->moveIndex) {
                const char *exitHint = "\ue001 Cancel";
                DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);
            } else {
                const char *exitHint = "\ue001 Back";
                DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);
            }

            DrawUtils::endDraw();
            redraw = false;
        }
    }
}

static bool ConfigItemDimensionsPad_callCallback(void *context, uint8_t index) {
    ConfigItemDimensionsPad *item = (ConfigItemDimensionsPad *) context;

    saveFavorites(item);

    if (item->callback && item->figureFiles[index]) {
        FILE *dimensionsFile = fopen(item->figureFiles[index].value().c_str(), "r+b");
        if (!dimensionsFile) {
            DEBUG_FUNCTION_LINE_ERR("Failed to open Dimensions file");
        } else {
            std::array<uint8_t, 0x2D * 0x04> fileData;
            const size_t ret_code = fread(fileData.data(), sizeof(fileData[0]), fileData.size(), dimensionsFile);
            if (ret_code == fileData.size()) {
                uint8_t pad = derivePadFromIndex(index);
                if (pad > 0) {
                    item->figureNumbers[index] = g_dimensionstoypad.LoadFigure(fileData, std::move(dimensionsFile), pad, index);
                }
            } else {
                DEBUG_FUNCTION_LINE_ERR("Dimensions file too small");
                fclose(dimensionsFile);
            }
        }
        item->callback(item, item->figureFiles[index].value().c_str(), index);
        return true;
    }

    return false;
}

static void ConfigItemDimensionsPad_onInput(void *context, WUPSConfigSimplePadData input) {
    ConfigItemDimensionsPad *item = (ConfigItemDimensionsPad *) context;

    if ((input.buttons_d & WUPS_CONFIG_BUTTON_A) == WUPS_CONFIG_BUTTON_A) {
        enterToypadMenu(item);
    }
}

static bool ConfigItemDimensionsPad_isMovementAllowed(void *context) {
    return true;
}

static int32_t ConfigItemDimensionsPad_getCurrentValueDisplay(void *context, char *out_buf, int32_t out_size) {
    strncpy(out_buf, "Manage Toypad Figures", out_size);
    return 0;
}

static void ConfigItemDimensionsPad_restoreDefault(void *context) {
    ConfigItemDimensionsPad *item = (ConfigItemDimensionsPad *) context;
    item->figureFiles.fill(std::nullopt);
    item->currentPath = item->rootPath;
}

static void ConfigItemDimensionsPad_onSelected(void *context, bool isSelected) {
}

bool ConfigItemDimensionsPad_AddToCategory(WUPSConfigCategoryHandle cat, const char *configID, const char *displayName, const char *dimensionsFolder, const char *currentFigure, DimensionsSelectedCallback callback) {
    if (!displayName || !dimensionsFolder || !currentFigure) {
        return false;
    }

    ConfigItemDimensionsPad *item = new ConfigItemDimensionsPad;
    if (!item) {
        return false;
    }

    item->callback = callback;
    item->rootPath = dimensionsFolder;

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
            .getCurrentValueDisplay         = &ConfigItemDimensionsPad_getCurrentValueDisplay,
            .getCurrentValueSelectedDisplay = &ConfigItemDimensionsPad_getCurrentValueDisplay,
            .onSelected                     = &ConfigItemDimensionsPad_onSelected,
            .restoreDefault                 = &ConfigItemDimensionsPad_restoreDefault,
            .isMovementAllowed              = &ConfigItemDimensionsPad_isMovementAllowed,
            .onInput                        = &ConfigItemDimensionsPad_onInput,
            .onDelete                       = &ConfigItemDimensionsPad_onDelete,
            //.onCloseCallback                = &ConfigItemDimensionsPad_callCallback,
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

static void ConfigItemDimensionsPad_onDelete(void *context) {
    ConfigItemDimensionsPad *item = (ConfigItemDimensionsPad *) context;

    free(item->configID);

    delete item;
}
