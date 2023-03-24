import QtQuick 2.12

Item {
    property alias colorMapPurity: themesColor.themePurity
    property alias colorMapTwilight: themesColor.themeTwilight

    property alias imageMap: themesImage

    ThemesColor {
        id: themesColor
    }

    ThemesImage {
        id: themesImage
    }
}
