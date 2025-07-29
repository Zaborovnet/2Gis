#include "fileprocessor.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QCoreApplication>
#include <algorithm>

FileProcessor::FileProcessor(QObject* parent)
    : QObject(parent)
{
}

FileProcessor::~FileProcessor()
{
  // Гарантируем остановку при уничтожении
  cancelProcessing();
}

void FileProcessor::startProcessing(QString filePath_str)
{
  // Блокировка для безопасного доступа
  QMutexLocker locker(&m_mutex_o);

  // Сброс состояния
  m_currentFilePath_str = filePath_str;
  m_paused_b = false;
  m_canceled_b = false;
  m_wordCount_map.clear();
  m_processingActive_b = true;

  // Асинхронный запуск обработки
  QMetaObject::invokeMethod(this, "processFile", Qt::QueuedConnection);
}

void FileProcessor::pauseProcessing()
{
  QMutexLocker locker(&m_mutex_o);
  if (!m_paused_b && !m_canceled_b) {
      m_paused_b = true;
    }
}

void FileProcessor::resumeProcessing()
{
  QMutexLocker locker(&m_mutex_o);
  if (m_paused_b && !m_canceled_b) {
      m_paused_b = false;
      m_pauseCondition_o.wakeAll();
    }
}

void FileProcessor::cancelProcessing()
{
  QMutexLocker locker(&m_mutex_o);
  if (m_processingActive_b) { // Проверять только если активно
      m_canceled_b = true;
      m_paused_b = false;
      m_processingActive_b = false; // Сбросить флаг активности
      m_pauseCondition_o.wakeAll();
      emit processingFinished(); // Гарантированно отправлять сигнал
    }
}

void FileProcessor::processFile()
{
  QFile file_o(m_currentFilePath_str);
  if (!file_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
      emit errorOccurred("Не удалось открыть файл: " + file_o.errorString());
      return;
    }

  QTextStream stream_o(&file_o);
  const qint64 totalSize_i = file_o.size();
  qint64 processedSize_i = 0;
  const qint64 chunkSize_i = 1024;

  QRegularExpression wordRegex_o("\\b\\w+\\b", QRegularExpression::UseUnicodePropertiesOption);

  while (!stream_o.atEnd() && m_processingActive_b) {
      // Обработка событий для реагирования на команды
      QCoreApplication::processEvents();

      // Проверка состояния с блокировкой мьютекса
      {
        QMutexLocker locker(&m_mutex_o);
        if (m_canceled_b) break;

        // Ожидание при паузе
        while (m_paused_b && !m_canceled_b) {
            m_pauseCondition_o.wait(&m_mutex_o);
          }
        if (m_canceled_b) break;
      }

      // Чтение блока данных
      QString buffer_str = stream_o.read(chunkSize_i);
      if (buffer_str.isEmpty()) break;
      processedSize_i += buffer_str.size();

      // Расчет и отправка прогресса
      int progress_i = totalSize_i > 0
                           ? static_cast<int>((processedSize_i * 100) / totalSize_i)
                           : 0;
      emit progressUpdated(progress_i);

      // Поиск и подсчет слов
      auto matchIterator_o = wordRegex_o.globalMatch(buffer_str);
      while (matchIterator_o.hasNext()) {
          QString word_str = matchIterator_o.next().captured().toLower();
          if (!word_str.isEmpty()) {
              QMutexLocker locker(&m_mutex_o);
              m_wordCount_map[word_str]++;
            }
        }

      // Периодическое обновление топ-слов
      if (processedSize_i % (chunkSize_i * 10) == 0) {
          emit topWordsReady(getTopWords(15));
        }
    }

  file_o.close();

  if (m_processingActive_b) { // Проверять флаг активности
      if (!m_canceled_b) {
          emit topWordsReady(getTopWords(15));
          emit processingFinished();
        }
      m_processingActive_b = false; // Сбросить флаг после завершения
    }
}

QVector<QPair<QString, int>> FileProcessor::getTopWords(int count_i) const
{
  QMutexLocker locker(&m_mutex_o);

  // Подготовка данных для сортировки
  QVector<QPair<QString, int>> words_vec;
  words_vec.reserve(m_wordCount_map.size());

  for (auto it = m_wordCount_map.constBegin(); it != m_wordCount_map.constEnd(); ++it) {
      words_vec.append(qMakePair(it.key(), it.value()));
    }

         // Частичная сортировка для топ-N элементов
  std::partial_sort(
      words_vec.begin(),
      words_vec.begin() + std::min(count_i, words_vec.size()),
      words_vec.end(),
      [](const auto& a, const auto& b) {
        return a.second > b.second;
      }
      );

         // Возврат только требуемого количества
  return words_vec.mid(0, std::min(count_i, words_vec.size()));
}
