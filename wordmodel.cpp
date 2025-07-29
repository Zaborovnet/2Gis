#include "wordmodel.h"

WordModel::WordModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int WordModel::rowCount(const QModelIndex& parent) const
{
  // Для списков родитель всегда невалиден
  return parent.isValid() ? 0 : m_data.size();
}

QVariant WordModel::data(const QModelIndex& index, int role) const
{
  // Проверка валидности индекса
  if (!index.isValid() || index.row() >= m_data.size()) {
      return {};
    }

         // Получение элемента данных
  const auto& item = m_data.at(index.row());

         // Возврат данных по ролям
  switch (static_cast<Role>(role)) {
    case Role::Word: return item.first;    // Слово
    case Role::Count: return item.second;  // Количество
    default: return {};
    }
}

QHash<int, QByteArray> WordModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[static_cast<int>(Role::Word)] = "word";
  roles[static_cast<int>(Role::Count)] = "count";
  return roles;
}

int WordModel::maxCount() const
{
  return m_maxCount_i;
}

void WordModel::updateData(QVector<QPair<QString, int>> data)
{
  // Начало сброса модели
  beginResetModel();

  // Обновление данных
  m_data = data;

  // Поиск нового максимального значения
  int newMaxCount_i = 1;
  for (const auto& item : m_data) {
      if (item.second > newMaxCount_i) {
          newMaxCount_i = item.second;
        }
    }

  // Обновление и отправка сигнала при изменении
  if (newMaxCount_i != m_maxCount_i) {
      m_maxCount_i = newMaxCount_i;
      emit maxCountChanged(m_maxCount_i);
    }

  // Завершение сброса модели
  endResetModel();
}
