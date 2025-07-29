#ifndef WORDMODEL_H
#define WORDMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief Модель данных для отображения статистики слов
 *
 * Предоставляет данные для гистограммы в формате, понятном для QML.
 * Хранит список слов и их частоту встречаемости.
 */
class WordModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int maxCount READ maxCount NOTIFY maxCountChanged)

      public:
               /**
                * @brief Роли для доступа к данным
                */
               enum class Role : int {
                 Word = Qt::UserRole + 1,  ///< Роль для получения слова
                 Count                     ///< Роль для получения количества
               };
  Q_ENUM(Role)

  /**
   * @brief Конструктор модели
   * @param parent Родительский объект
   */
  explicit WordModel(QObject* parent = nullptr);

  /**
   * @brief Количество строк в модели
   * @param parent Индекс родителя
   * @return Количество элементов
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * @brief Получение данных по ролям
   * @param index Индекс элемента
   * @param role Роль данных
   * @return Запрошенные данные
   */
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  /**
   * @brief Получение списка ролей
   * @return Хеш ролей
   */
  QHash<int, QByteArray> roleNames() const override;

  /**
   * @brief Получение максимального количества повторений
   * @return Максимальное количество повторений слова
   */
  int maxCount() const;

public slots:
  /**
   * @brief Обновление данных модели
   * @param data Вектор пар (слово, количество)
   */
  void updateData(QVector<QPair<QString, int>> data);

signals:
  /**
   * @brief Сигнал об изменении максимального количества
   * @param maxCount Новое максимальное значение
   */
  void maxCountChanged(int maxCount);

private:
  QVector<QPair<QString, int>> m_data;  ///< Данные модели
  int m_maxCount_i = 1;                 ///< Максимальное количество повторений
};

#endif // WORDMODEL_H
