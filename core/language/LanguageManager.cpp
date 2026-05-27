#include "language/LanguageManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Vitals {

namespace {

constexpr auto RuntimeLanguagesDir = "languages";
constexpr auto ManifestFileName = "languages.json";
constexpr auto ResourceLanguagesPrefix = ":/languages";
constexpr auto FallbackLanguage = "en-US";

} // namespace

LanguageManager::LanguageManager(QObject* parent)
    : QObject(parent)
{
}

bool LanguageManager::initialize(const QString& preferredLanguage)
{
    if (!loadManifest()) {
        return false;
    }

    const QString requested = preferredLanguage.isEmpty() ? m_defaultLanguage : preferredLanguage;
    if (setLanguage(requested)) {
        return true;
    }
    if (setLanguage(m_defaultLanguage)) {
        return true;
    }
    return !m_languages.isEmpty() && setLanguage(m_languages.first().code);
}

QList<LanguageManager::Language> LanguageManager::availableLanguages() const
{
    return m_languages;
}

QString LanguageManager::currentLanguage() const
{
    return m_currentLanguage;
}

bool LanguageManager::setLanguage(const QString& languageCode)
{
    const Language language = m_languageByCode.value(languageCode);
    if (language.code.isEmpty() || !loadDictionary(language)) {
        return false;
    }

    if (m_currentLanguage == language.code) {
        return true;
    }

    m_currentLanguage = language.code;
    Q_EMIT languageChanged(language.code);
    return true;
}

QString LanguageManager::translate(const QString& key, const QString& fallback) const
{
    const QString value = m_dictionary.value(key);
    if (!value.isEmpty()) {
        return value;
    }
    return fallback.isEmpty() ? key : fallback;
}

bool LanguageManager::loadManifest()
{
    const QJsonObject root = QJsonDocument::fromJson(readAll(manifestPath())).object();
    const QJsonArray languages = root.value(QStringLiteral("languages")).toArray();
    if (languages.isEmpty()) {
        return false;
    }

    m_languages.clear();
    m_languageByCode.clear();
    m_defaultLanguage = root.value(QStringLiteral("defaultLanguage")).toString(QString::fromLatin1(FallbackLanguage));

    for (const QJsonValue& value : languages) {
        const QJsonObject object = value.toObject();
        Language language;
        language.code = object.value(QStringLiteral("code")).toString();
        language.name = object.value(QStringLiteral("name")).toString();
        language.nativeName = object.value(QStringLiteral("nativeName")).toString(language.name);
        language.fileName = object.value(QStringLiteral("file")).toString();
        if (language.code.isEmpty() || language.fileName.isEmpty()) {
            continue;
        }
        m_languages.append(language);
        m_languageByCode.insert(language.code, language);
    }

    return !m_languages.isEmpty();
}

bool LanguageManager::loadDictionary(const Language& language)
{
    QHash<QString, QString> dictionary = dictionaryFromJson(readAll(dictionaryPath(language.fileName)));
    if (dictionary.isEmpty() && language.code != QString::fromLatin1(FallbackLanguage)) {
        const Language fallback = m_languageByCode.value(QString::fromLatin1(FallbackLanguage));
        if (!fallback.fileName.isEmpty()) {
            dictionary = dictionaryFromJson(readAll(dictionaryPath(fallback.fileName)));
        }
    }

    if (dictionary.isEmpty()) {
        return false;
    }

    m_dictionary = dictionary;
    return true;
}

QString LanguageManager::manifestPath() const
{
    const QString runtimePath = QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("%1/%2").arg(QString::fromLatin1(RuntimeLanguagesDir), QString::fromLatin1(ManifestFileName)));
    if (QFile::exists(runtimePath)) {
        return runtimePath;
    }
    return QStringLiteral("%1/%2").arg(QString::fromLatin1(ResourceLanguagesPrefix), QString::fromLatin1(ManifestFileName));
}

QString LanguageManager::dictionaryPath(const QString& fileName) const
{
    const QString runtimePath = QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("%1/%2").arg(QString::fromLatin1(RuntimeLanguagesDir), fileName));
    if (QFile::exists(runtimePath)) {
        return runtimePath;
    }
    return QStringLiteral("%1/%2").arg(QString::fromLatin1(ResourceLanguagesPrefix), fileName);
}

QHash<QString, QString> LanguageManager::dictionaryFromJson(const QByteArray& jsonData)
{
    QHash<QString, QString> dictionary;
    const QJsonObject root = QJsonDocument::fromJson(jsonData).object();
    const QJsonObject translations = root.value(QStringLiteral("translations")).toObject();
    for (auto it = translations.begin(); it != translations.end(); ++it) {
        dictionary.insert(it.key(), it.value().toString());
    }
    return dictionary;
}

QByteArray LanguageManager::readAll(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

} // namespace Vitals
