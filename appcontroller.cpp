#include "appcontroller.h"
#include "wordmodel.h"
#include "fileprocessor.h"
#include <QMessageBox>

AppController::AppController(QObject* parent)
    : QObject(parent)
{
  // Инициализация модели
  m_wordModel_po = new WordModel(this);

  // Создание обработчика и потока
  m_processor_po = new FileProcessor();
  m_workerThread_po = new QThread();

  // Перемещение обработчика в поток
  m_processor_po->moveToThread(m_workerThread_po);

  // Соединение сигналов и слотов
  connect(m_processor_po, &FileProcessor::progressUpdated,
           this, &AppController::handleProgress);
  connect(m_processor_po, &FileProcessor::topWordsReady,
           this, &AppController::handleTopWords);
  connect(m_processor_po, &FileProcessor::processingFinished,
           this, &AppController::handleFinished);
  connect(m_processor_po, &FileProcessor::errorOccurred,
           this, &AppController::handleError);

  // Управление жизненным циклом
  connect(m_workerThread_po, &QThread::finished,
           m_processor_po, &QObject::deleteLater);
  connect(m_workerThread_po, &QThread::finished,
           m_workerThread_po, &QObject::deleteLater);

  // Запуск рабочего потока
  m_workerThread_po->start();
}

AppController::~AppController()
{
  // Остановка обработки и потока
  cancelProcessing();
  m_workerThread_po->quit();
  m_workerThread_po->wait();
}

int AppController::progress() const
{
  return m_progress_i;
}

bool AppController::isProcessing() const
{
  return m_isProcessing_b;
}

bool AppController::isPaused() const
{
  return m_isPaused_b;
}

WordModel* AppController::wordModel() const
{
  return m_wordModel_po;
}

void AppController::openFile(QString filePath_str)
{
  // Отменять текущую обработку при открытии нового файла
  if (m_isProcessing_b) {
      cancelProcessing();
    }
  m_currentFile_str = filePath_str;}

void AppController::startProcessing()
{
  // Проверка наличия файла
  if (m_currentFile_str.isEmpty()) {
      emit errorOccurred("Файл не выбран");
      return;
    }

  // Всегда отменять предыдущую обработку перед запуском новой
  if (m_isProcessing_b) {
      cancelProcessing();
    }

  resetState();
  m_isProcessing_b = true;
  emit isProcessingChanged(m_isProcessing_b);

  // Асинхронный вызов обработки
  QMetaObject::invokeMethod(m_processor_po, "startProcessing",
                             Qt::QueuedConnection,
                             Q_ARG(QString, m_currentFile_str));
}

void AppController::pauseProcessing()
{
  if (m_isProcessing_b && !m_isPaused_b) {
      m_isPaused_b = true;
      emit isPausedChanged(m_isPaused_b);
      QMetaObject::invokeMethod(m_processor_po, "pauseProcessing", Qt::QueuedConnection);
    }
}

void AppController::resumeProcessing()
{
  if (m_isProcessing_b && m_isPaused_b) {
      m_isPaused_b = false;
      emit isPausedChanged(m_isPaused_b);
      QMetaObject::invokeMethod(m_processor_po, "resumeProcessing", Qt::QueuedConnection);
    }
}

void AppController::cancelProcessing()
{
  if (m_isProcessing_b) {
      // Сначала отправить команду в поток
      QMetaObject::invokeMethod(m_processor_po, "cancelProcessing", Qt::QueuedConnection);

      // Ожидаем подтверждения от потока
      QMutex mutex;
      QWaitCondition waitCondition;
      QMutexLocker locker(&mutex);
      waitCondition.wait(&mutex, 100); // Краткая задержка

      // Сбросить состояние UI
      resetState();

      // Очистить модель данных
      m_wordModel_po->updateData(QVector<QPair<QString, int>>());
      emit dataReset();
    }
}

void AppController::handleProgress(int percentage_i)
{
  if (m_progress_i != percentage_i) {
      m_progress_i = percentage_i;
      emit progressChanged(m_progress_i);
    }
}

void AppController::handleTopWords(QVector<QPair<QString, int>> topWords)
{
  m_wordModel_po->updateData(topWords);
}

void AppController::handleFinished()
{
  resetState();
}

void AppController::handleError(QString message)
{
  resetState();
  // Очистить модель данных при ошибке
  m_wordModel_po->updateData(QVector<QPair<QString, int>>());
  emit errorOccurred(message);}

void AppController::resetState()
{
  if (m_isProcessing_b || m_isPaused_b || m_progress_i > 0) {
      m_isProcessing_b = false;
      m_isPaused_b = false;
      m_progress_i = 0;

      emit isProcessingChanged(m_isProcessing_b);
      emit isPausedChanged(m_isPaused_b);
      emit progressChanged(m_progress_i);
    }
}
