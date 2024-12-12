#pragma once

#include "highlightoptions.h"
#include "pathhelper.h"

#include <QBitArray>
#include <QSettings>

/**
 * @brief The MainWindowSettings structure wraps the saved settings for the
 * application main window.
 */
struct MainWindowSettings {
    QByteArray geometry;
    QByteArray windowState;
    QStringList recentFiles;
    QString lastOpenFolder;
};

class AppOptions {
private:
    AppOptions()
        : m_settings(PathHelper::GetConfigIniPath(), QSettings::IniFormat)
    {
        ReadSettings();
    }
    AppOptions(const AppOptions &) = delete;
    AppOptions &operator=(const AppOptions &) = delete;

    QStringList m_skippedText;
    QBitArray m_skippedState;
    bool m_visualizationServiceEnable;
    QString m_visualizationServiceURL;
    QString m_diffToolPath;
    bool m_futureTabsUnderLive;
    bool m_captureAllTextFiles;
    bool m_showArtDataInValue;
    bool m_showErrorCodeInValue;
    QString m_defaultFilterName;
    HighlightOptions m_defaultHighlightOpts;
    int m_syntaxHighlightLimit;
    QString m_theme;
    QString m_notation;
    QSettings m_settings;

public:
    static AppOptions &GetInstance()
    {
        static AppOptions options;
        return options;
    }

    QSettings &getSettings();

    void ReadSettings();
    void WriteSettings();
    void LoadHighlightFilter(const QString &filterName);

    MainWindowSettings readMainWindowSettings();
    void saveMainWindowSettings(const MainWindowSettings &windowSettings);

    QStringList getSkippedText() const;
    void setSkippedText(const QStringList &skippedText);

    QBitArray getSkippedState() const;
    void setSkippedState(const QBitArray &skippedState);

    bool getVisualizationServiceEnable() const;
    void setVisualizationServiceEnable(const bool visualizationServiceEnable);

    QString getVisualizationServiceURL() const;
    void setVisualizationServiceURL(const QString &visualizationServiceURL);

    QString getDiffToolPath() const;
    void setDiffToolPath(const QString &diffToolPath);

    bool getFutureTabsUnderLive() const;
    void setFutureTabsUnderLive(const bool futureTabsUnderLive);

    bool getCaptureAllTextFiles() const;
    void setCaptureAllTextFiles(const bool captureAllTextFiles);

    bool getShowArtDataInValue() const;
    void setShowArtDataInValue(const bool showArtDataInValue);

    bool getShowErrorCodeInValue() const;
    void setShowErrorCodeInValue(const bool showErrorCodeInValue);

    QString getDefaultFilterName() const;
    void setDefaultFilterName(const QString &defaultFilterName);

    HighlightOptions getDefaultHighlightOpts();

    int getSyntaxHighlightLimit() const;
    void setSyntaxHighlightLimit(const int syntaxHighlightLimit);

    QString getTheme() const;
    void setTheme(const QString &theme);

    QString getNotation() const;
    void setNotation(const QString &notation);
};
