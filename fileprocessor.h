#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QPair>
#include <QMutex>
#include <QWaitCondition>
#include <QHash>

// Предварительное объявление классов
class QFile;
class QTextStream;
class QRegularExpression;

/**
 * @brief Класс для обработки файлов в отдельном потоке
 *
 * Выполняет чтение файла, подсчет слов и формирование статистики.
 * Работает в отдельном потоке для обеспечения отзывчивости UI.
 */
class FileProcessor : public QObject
{
  Q_OBJECT

public:
   /**
    * @brief Конструктор процессора
    * @param parent Родительский объект
    */
   explicit FileProcessor(QObject* parent = nullptr);

  /**
   * @brief Деструктор
   */
  ~FileProcessor();

public slots:
  /**
   * @brief Запуск обработки файла
   * @param filePath_str Путь к файлу
   */
  void startProcessing(QString filePath_str);

  /**
   * @brief Приостановка обработки
   */
  void pauseProcessing();

  /**
   * @brief Возобновление обработки
   */
  void resumeProcessing();

  /**
   * @brief Отмена обработки
   */
  void cancelProcessing();

signals:
  /**
   * @brief Обновление прогресса обработки
   * @param percentage_i Процент выполнения (0-100)
   */
  void progressUpdated(int percentage_i);

  /**
   * @brief Готовность топ-слов
   * @param topWords Вектор пар (слово, количество)
   */
  void topWordsReady(QVector<QPair<QString, int>> topWords);

  /**
   * @brief Завершение обработки
   */
  void processingFinished();

  /**
   * @brief Ошибка при обработке
   * @param message Сообщение об ошибке
   */
  void errorOccurred(QString message);

private:
  /**
   * @brief Основной метод обработки файла
   */
  Q_INVOKABLE void processFile();

  /**
   * @brief Получение топ-N слов
   * @param count_i Количество слов
   * @return Вектор пар (слово, количество)
   */
  QVector<QPair<QString, int>> getTopWords(int count_i) const;

  QString m_currentFilePath_str;       ///< Текущий обрабатываемый файл
  QHash<QString, int> m_wordCount_map; ///< Счетчик слов
  bool m_paused_b = false;             ///< Флаг паузы
  bool m_canceled_b = false;           ///< Флаг отмены
  mutable QMutex m_mutex_o;            ///< Мьютекс для синхронизации
  QWaitCondition m_pauseCondition_o;   ///< Условие для паузы
  bool m_processingActive_b = false;   ///< Флаг активной обработки
  bool m_stopRequested_b = false;      ///< Флаг запроса остановки

};

#endif // FILEPROCESSOR_H
