#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QThread>

// Предварительные объявления
class WordModel;
class FileProcessor;

/**
 * @brief Главный контроллер приложения
 *
 * Связывает UI, модель данных и обработчик файлов.
 * Управляет потоками и обрабатывает сигналы.
 */
class AppController : public QObject
{
  Q_OBJECT
  Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
      Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)
      Q_PROPERTY(bool isPaused READ isPaused NOTIFY isPausedChanged)
      Q_PROPERTY(WordModel* wordModel READ wordModel CONSTANT)

      public:
               /**
                * @brief Конструктор контроллера
                * @param parent Родительский объект
                */
               explicit AppController(QObject* parent = nullptr);

  /**
   * @brief Деструктор
   */
  ~AppController();

  /**
   * @brief Получение текущего прогресса
   * @return Прогресс в процентах
   */
  int progress() const;

  /**
   * @brief Проверка выполнения обработки
   * @return true если обработка активна
   */
  bool isProcessing() const;

  /**
   * @brief Проверка состояния паузы
   * @return true если обработка на паузе
   */
  bool isPaused() const;

  /**
   * @brief Получение модели данных
   * @return Указатель на модель слов
   */
  WordModel* wordModel() const;

public slots:
  /**
   * @brief Открытие файла для обработки
   * @param filePath_str Путь к файлу
   */
  void openFile(QString filePath_str);

  /**
   * @brief Запуск обработки файла
   */
  void startProcessing();

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
   * @brief Изменение прогресса обработки
   * @param progress Новое значение прогресса
   */
  void progressChanged(int progress);

  /**
   * @brief Изменение состояния обработки
   * @param isProcessing true если обработка активна
   */
  void isProcessingChanged(bool isProcessing);

  /**
   * @brief Изменение состояния паузы
   * @param isPaused true если обработка на паузе
   */
  void isPausedChanged(bool isPaused);

  /**
   * @brief Ошибка при обработке
   * @param message Сообщение об ошибке
   */
  void errorOccurred(QString message);

  /**
   * @brief Сигнал сброса данных
   */
  void dataReset();

private slots:
  /**
   * @brief Обработчик обновления прогресса
   * @param percentage_i Значение прогресса
   */
  void handleProgress(int percentage_i);

  /**
   * @brief Обработчик готовности топ-слов
   * @param topWords Вектор пар (слово, количество)
   */
  void handleTopWords(QVector<QPair<QString, int>> topWords);

  /**
   * @brief Обработчик завершения обработки
   */
  void handleFinished();

  /**
   * @brief Обработчик ошибок
   * @param message Сообщение об ошибке
   */
  void handleError(QString message);

private:
  /**
   * @brief Сброс состояния контроллера
   */
  void resetState();

  WordModel* m_wordModel_po = nullptr;         ///< Модель данных
  FileProcessor* m_processor_po = nullptr;     ///< Обработчик файлов
  QThread* m_workerThread_po = nullptr;        ///< Рабочий поток
  QString m_currentFile_str;                   ///< Текущий файл
  int m_progress_i = 0;                        ///< Текущий прогресс
  bool m_isProcessing_b = false;               ///< Флаг обработки
  bool m_isPaused_b = false;                   ///< Флаг паузы
};

#endif // APPCONTROLLER_H
