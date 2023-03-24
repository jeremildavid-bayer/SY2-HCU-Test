var DATE_TIME_MIN_VALUE = "0001-01-01T00:00:00.0000000Z";

function getFontPixel(point)
{
    return point;
}

function getPixelH(pixel)
{
    return (pixel / 1920) * dsCfgLocal.screenW;
}

function getPixelV(pixel)
{
    return (pixel / 1200) * dsCfgLocal.screenH;
}

function copyObject(from)
{
    if (from === undefined)
    {
        return undefined;
    }
    else
    {
        //logDebug("copyObject().from=" + JSON.stringify(from));
        return JSON.parse(JSON.stringify(from));
    }
}

function compareObjects(src1, src2)
{
    return JSON.stringify(src1) == JSON.stringify(src2);
}

function swapListItem(list, idx1, idx2)
{
    var itemBuf = list[idx1];
    list[idx1] = list[idx2];
    list[idx2] = itemBuf;
    return list;
}

function getMinimisedDurationStrHours(srcStr)
{
    // Convert from "hh:mm:ss.zzz" to "hh:mm"
    var strList = srcStr.split(":");
    if (strList.length >= 3)
    {
        var durationStr = strList[0] + ":" + strList[1];
        return durationStr;
    }
    return srcStr;
}

function durationStrToHumanReadableFormatStr(srcStr)
{
    // Convert from "<days>.<hours>:<mins>" to "<days>d <hours>h <min>m"
    var strList = srcStr.split(":");
    if (strList.length >= 3)
    {
        var durationStr = "";
        var seconds = parseInt(strList[2]);
        var minutes = parseInt(strList[1]);
        var day = 0;
        var hour = parseInt(strList[0]);
        var dayAndHourList = strList[0].split(".");
        if (dayAndHourList.length > 1)
        {
            day = parseInt(dayAndHourList[0]);
            hour = parseInt(dayAndHourList[1]);
        }

        var hours = parseInt(strList[0]);

        if (day > 0)
        {
            durationStr = day.toString() + translate("T_Units_d") + " " + hour.toString() + translate("T_Units_h") + " " + minutes.toString() + translate("T_Units_m");
        }
        else if (hour > 0)
        {
            durationStr = hour.toString() + translate("T_Units_h") + " " + minutes.toString() + translate("T_Units_m");
        }
        else if (minutes > 0)
        {
            durationStr = minutes.toString() + translate("T_Units_m");
        }
        else if (seconds > 0)
        {
            durationStr = seconds.toString() + translate("T_Units_s");
        }
        else
        {
            // Round up to 1s
            durationStr = "1" + translate("T_Units_s");
        }

        return durationStr;
    }

    return srcStr;
}

function getMinimisedDurationStr(srcStr)
{
    // Convert from "hh:mm:ss.zzz" to "mm:ss"
    var strList = srcStr.split(":");
    if (strList.length >= 3)
    {
        var hour = parseInt(strList[0]);
        var min = parseInt(strList[1]) + (hour * 60);
        var secStr = strList[2].split(".")[0];
        var minStr = min.toString();
        if (minStr.length < 2)
        {
            minStr = "0" + minStr;
        }
        var durationStr = minStr + ":" + secStr;
        return durationStr;
    }
    return srcStr;
}

function getFormattedDurationStr(srcStr, formatStr)
{
    // Convert "hh:mm:ss.zzz" according to formatStr
    // formatStr supports "h(H):mm:ss" where h:mm:ss will force show h, H:mm:ss will hide H when it's zero

    var strList = srcStr.split(":");
    if (strList.length >= 3)
    {
        var formatStrList = formatStr.split(":");
        if (formatStrList.length !== 3)
        {
            return srcStr;
        }

        var hideZeroHour = (formatStrList[0].indexOf("H") !== -1);

        var hourStr = strList[0];
        if (hourStr.length < 2)
        {
            hourStr = "0" + hourStr;
        }
        if (hideZeroHour && (hourStr === "00"))
        {
            hourStr = "";
        }

        var minStr = strList[1];
        var secStr = strList[2].split(".")[0];
        if (minStr.length < 2)
        {
            minStr = "0" + minStr;
        }

        var durationStr = minStr + ":" + secStr;
        if (hourStr !== "")
        {
            durationStr = hourStr + ":" + durationStr;
        }

        return durationStr;
    }
    return srcStr;
}

function millisecToDurationStr(msec)
{
    var sec = Math.floor(msec / 1000);
    msec = msec % 1000;
    var min =  Math.floor(sec / 60);
    sec = sec % 60;
    var hour =  Math.floor(min / 60);
    min = min % 60;
    var day = Math.floor(hour / 24);
    hour = hour % 24;

    msec = msec.toString();
    sec = sec.toString();
    min = min.toString();
    hour = hour.toString();
    day = day.toString();

    while (msec.length < 3)
    {
        msec = "0" + msec;
    }

    while (sec.length < 2)
    {
        sec = "0" + sec;
    }

    while (min.length < 2)
    {
        min = "0" + min;
    }

    while (hour.length < 2)
    {
        hour = "0" + hour;
    }

    return ((day === "0") ? "" : (day + ".")) + hour + ":" + min + ":" + sec + "." + msec.substring(0, 3);
}

function durationStrToMillisec(durationStr)
{
    var strList = durationStr.split(":");
    if (strList.length > 2)
    {
        // durationStr format: "hh:mm:ss.zzz";
        var day = 0;
        var hour = parseInt(strList[0]);
        var dayAndHourList = strList[0].split(".");
        if (dayAndHourList.length > 1)
        {
            day = parseInt(dayAndHourList[0]);
            hour = parseInt(dayAndHourList[1]);
        }

        var min = parseInt(strList[1]);
        var secAndmsecList = strList[2].split(".");
        var sec = parseInt(secAndmsecList[0]);
        var msec = (secAndmsecList.length > 1) ? parseInt(secAndmsecList[1]) : 0;
        return (msec) + (sec * 1000) + (min * 60 * 1000) + (hour * 60 * 60 * 1000) + (day * 60 * 60 * 1000 * 24);
    }
    else
    {
        // durationStr format: "12:34"
        var strBuf = durationStr;
        strBuf = strBuf.replace(/-/g, "0");
        strBuf = strBuf.replace(/ /g, "0");
        var minutes = parseInt(strBuf.substr(0, 2));
        var seconds = parseInt(strBuf.substr(3, 2));
        return (seconds + (minutes * 60)) * 1000;
    }
}

function getPressure(pressureUnit, pressureKpa)
{
    var pressureOut;

    if (pressureUnit === "psi")
    {
        pressureOut = pressureKpa * 0.14503773;
    }
    else if (pressureUnit === "kg/cm2")
    {
        pressureOut = pressureKpa * 0.0101972;
    }
    else
    {
        pressureOut = pressureKpa / 1.0;
    }

    return pressureOut;
}

function getPressureKpa(pressureUnit, pressure)
{
    var pressureOut;

    if (pressureUnit === "psi")
    {
        pressureOut = pressure / 0.14503773;
    }
    else if (pressureUnit === "kg/cm2")
    {
        pressureOut = pressure / 0.0101972;
    }
    else
    {
        pressureOut = pressure / 1.0;
    }

    return pressureOut;
}

function getTemperature(temperatureUnit, temperatureCelcius)
{
    var temperatureOut = temperatureCelcius;

    if (temperatureUnit === "degreesF")
    {
        temperatureOut = ((temperatureCelcius * 9) / 5) + 32;
    }
    return temperatureOut;
}

function utcDateTimeToMillisec(utcDateTimeStr)
{
    // "yyyy-MM-ddTHH:mm:ss.zzzZ" -> XXXX (ms)
    return Date.parse(utcDateTimeStr);
}

function utcDateTimeToExpiryFormat(utcDateTimeStr)
{
    // "yyyy-MM-ddTHH:mm:ss.zzzZ" -> "yyyy/MM"
    // applies UTC offset
    return utcDateTimeToDateTimeStr("yyyy/MM", "", dsCfgLocal.currentUtcOffsetMinutes, utcDateTimeStr);
}

function localeDateFormattoUTCDateTime(dateFormat, dateStr)
{
    //Locale date format -> Date Object
    //localeDateFormattoUTCDateTime.toISOString(dateFormat, dateStr) -> "yyyy-MM-ddTHH:mm:ss.zzzZ"
    //Expected format of valueFormat is yyyy/MM/dd hh:mm. The order of yyyy MM dd depends on locale settings
    // NOTE: according to javascript Date api documentation, month is 0 base index, default value for day is 1
    var year = 0, month = 0, day = 1, hour = 0, min = 0;
    var dateFormattoUtcObj;
    for (var index = 0; index < dateStr.length; index++)
    {
        if (dateFormat[index] === "y")
        {
            year = parseInt(dateStr.substring(index, index+4));
            index += 4;
        }
        else if (dateFormat[index] === "M")
        {
            month = parseInt(dateStr.substring(index, index+2));
            index += 2;
        }
        else if (dateFormat[index] === "d")
        {
            day = parseInt(dateStr.substring(index, index+2));
            index += 2;
        }
        else if (dateFormat[index] === "h")
        {
            hour = parseInt(dateStr.substring(index, index+2));
            index += 2;
        }
        else if (dateFormat[index] === "m")
        {
            min = parseInt(dateStr.substring(index, index+2));
            index += 2;
        }
    }
    dateFormattoUtcObj = new Date(year, month - 1, day, hour, min, 0);
    return dateFormattoUtcObj;
}

function utcDateTimeToDateTimeStr(dateFormat, timeFormat, currentUtcOffsetMinutes, utcDateTimeStr)
{
    var epochMs = utcDateTimeToMillisec(utcDateTimeStr);
    return getDateTimeStrFromEpochMs(dateFormat, timeFormat, currentUtcOffsetMinutes, epochMs);
}

function getCurrentUtcDateTime()
{
    var currentDate = new Date();
    var timezone = currentDate.getTimezoneOffset();
    var offsetHours = timezone / 60;
    var offsetMinutes = timezone % 60;
    currentDate.setHours(currentDate.getHours() + offsetHours);
    currentDate.setMinutes(currentDate.getMinutes() + offsetMinutes);
    return currentDate;
}

function getCurrentDateTimeStr(dateFormat, timeFormat, currentUtcOffsetMinutes)
{
    var dateObj = getCurrentUtcDateTime();
    return getDateTimeStr(dateFormat, timeFormat, currentUtcOffsetMinutes, dateObj);
}

function getDateTimeStrFromEpochMs(dateFormat, timeFormat, currentUtcOffsetMinutes, epochMs)
{
    var dateObj = new Date(epochMs);
    return getDateTimeStr(dateFormat, timeFormat, currentUtcOffsetMinutes, dateObj);
}

function getDateTimeStr(dateFormat, timeFormat, currentUtcOffsetMinutes, dateObj)
{
    var curLocale = appMain.locale;

    timeFormat = timeFormat.replace("tt", "AP");
    var format = dateFormat + " " + timeFormat;

    var offsetHours = currentUtcOffsetMinutes / 60;
    offsetHours = Math.floor(offsetHours);
    var offsetMinutes = currentUtcOffsetMinutes % 60;

    dateObj.setHours(dateObj.getHours() + offsetHours);
    dateObj.setMinutes(dateObj.getMinutes() + offsetMinutes);

    var dateStr, timeStr;

    // Handle QT bug on toLocalTimeString
    if ( (curLocale.name === "pt_PT") ||
         (curLocale.name === "es_ES") ||
         (curLocale.name === "fr_CH") )
    {
        var defaultLocale = Qt.locale("en_US");
        dateStr = dateObj.toLocaleDateString(defaultLocale, dateFormat);
        timeStr = dateObj.toLocaleTimeString(defaultLocale, timeFormat);

        if (curLocale.name === "fr_CH")
        {
            dateStr = dateStr.replace(/\//g, ".");
        }
    }
    else
    {
        // Use QT locale api
        dateStr = dateObj.toLocaleDateString(curLocale, dateFormat);

        if (curLocale.name === "nl_BE")
        {
            dateStr = dateStr.replace(/-/g, "/");
        }
        else if (curLocale.name === "fr_CA" || curLocale.name === "fr_BE")
        {
            dateStr = dateStr.replace(/\//g, "-");
        }
        else if (curLocale.name === "kk_KZ" || curLocale.name === "et_EE" || curLocale.name === "it_CH")
        {
            dateStr = dateStr.replace(/\//g, ".");
        }
        else if (curLocale.name === "sk_SK" || curLocale.name === "sl_SI")
        {
            dateStr = dateStr.replace(/\//g, ". ");
        }

        timeStr = dateObj.toLocaleTimeString(curLocale, timeFormat);
    }

    var dateTimeStr = dateStr + (((dateStr !== "") && (timeStr !== "")) ? " " : "") + timeStr;
    return dateTimeStr;
}

function getFluidMaximumUseDurationMs(fluidSource)
{
    if ( (fluidSource !== undefined) &&
         (fluidSource.SourcePackages[0] !== undefined) &&
         (fluidSource.SourcePackages[0].MaximumUseDuration !== undefined) )
    {
        return durationStrToMillisec(fluidSource.SourcePackages[0].MaximumUseDuration);
    }
    return -1;
}

function getIntersection(rect1, rect2)
{
    if (rect1.x > rect2.x + rect2.width) return null;
    if (rect1.x + rect1.width < rect2.x) return null;
    if (rect1.y > rect2.y + rect2.height) return null;
    if (rect1.y + rect1.height < rect2.y) return null;

    var rect = {};
    rect.x = Math.max(rect1.x, rect2.x);
    rect.y = Math.max(rect1.y, rect2.y);
    rect.width = Math.min(rect1.x + rect1.width, rect2.x + rect2.width) - rect.x;
    rect.height = Math.min(rect1.y + rect1.height, rect2.y + rect2.height) - rect.y;

    return rect;
}

function getDecimalDigits(doubleStr)
{
    var doubleStrSplit = doubleStr.split(".");
    return (doubleStrSplit.length <= 1) ? 0 : doubleStrSplit[1].length;
}
