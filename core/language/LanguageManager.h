#pragma once

#include <QHash>
#include <QObject>
#include <QString>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief JSON-backed language catalog for host UI text
 *
 * Loads a language manifest and flat translation dictionaries from the runtime
 * languages directory, falling back to compiled resources. New languages can be
 * added by dropping another JSON dictionary and registering it in the manifest.
 * \endif
 *
 * \if CHINESE
 * @brief 基于 JSON 的宿主界面语言目录
 *
 * 从运行目录 languages 加载语言清单与扁平化翻译字典，并在缺失时回退到编译
 * 资源。新增语言时只需要放入新的 JSON 字典并在清单中登记。
 * \endif
 */
class LanguageManager : public QObject
{
    Q_OBJECT

public:
    struct Language
    {
        QString code;
        QString name;
        QString nativeName;
        QString fileName;
    };

    explicit LanguageManager(QObject* parent = nullptr);

    /**
     * \if ENGLISH
     * @brief Loads the manifest and selects the requested language when present
     * \endif
     *
     * \if CHINESE
     * @brief 加载语言清单，并在存在时切换到指定语言
     * \endif
     */
    bool initialize(const QString& preferredLanguage);

    QList<Language> availableLanguages() const;
    QString currentLanguage() const;

    /**
     * \if ENGLISH
     * @brief Switches to a language code declared by the JSON manifest
     * \endif
     *
     * \if CHINESE
     * @brief 切换到 JSON 清单中声明的语言代码
     * \endif
     */
    bool setLanguage(const QString& languageCode);

    /**
     * \if ENGLISH
     * @brief Resolves a translation key with a fallback string
     * \endif
     *
     * \if CHINESE
     * @brief 通过 key 获取翻译，缺失时返回回退文案
     * \endif
     */
    QString translate(const QString& key, const QString& fallback = QString()) const;

Q_SIGNALS:
    void languageChanged(const QString& languageCode);

private:
    bool loadManifest();
    bool loadDictionary(const Language& language);
    QString manifestPath() const;
    QString dictionaryPath(const QString& fileName) const;
    static QHash<QString, QString> dictionaryFromJson(const QByteArray& jsonData);
    static QByteArray readAll(const QString& path);

    QList<Language> m_languages;
    QHash<QString, Language> m_languageByCode;
    QHash<QString, QString> m_dictionary;
    QString m_currentLanguage;
    QString m_defaultLanguage;
};

} // namespace Vitals
