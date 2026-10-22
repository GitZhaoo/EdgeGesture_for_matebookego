#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QQmlEngine>
#include <QTranslator>
#include <QVariantMap>

class Translator : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE
                 setCurrentLanguage NOTIFY currentLanguageChanged)
  Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT)
  // Expose the translation table to QML as a property map keyed by source
  // string. QML accesses via Translator.tr("source").
  Q_PROPERTY(QVariantMap strings READ strings NOTIFY currentLanguageChanged)

public:
  explicit Translator(QObject *parent = nullptr);

  QString currentLanguage() const { return m_currentLanguage; }
  void setCurrentLanguage(const QString &lang);

  QStringList availableLanguages() const { return m_availableLanguages; }

  QVariantMap strings() const { return m_strings; }

  // QML-invokable translate function: Translator.tr("English Text")
  Q_INVOKABLE QString tr(const QString &source) const;

  // Persist language choice to a tiny config file next to the exe.
  void loadSavedLanguage();
  void saveLanguage(const QString &lang);

signals:
  void currentLanguageChanged();

private:
  void loadLanguageData(const QString &lang);

  QString m_currentLanguage = QStringLiteral("zh_CN");
  QStringList m_availableLanguages = {"zh_CN", "en_US"};
  QVariantMap m_strings;
};

// Singleton provider for QML
static QObject *translatorProvider(QQmlEngine *engine, QJSEngine *scriptEngine) {
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  static Translator *t = nullptr;
  if (!t)
    t = new Translator();
  return t;
}

#endif // TRANSLATOR_H
