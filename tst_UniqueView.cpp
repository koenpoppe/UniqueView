#include <QtTest>
#include <vector>

#include "UniqueView.h"

#ifdef QT_DEBUG
#define MYBENCHMARK QBENCHMARK_ONCE
#else
#define MYBENCHMARK QBENCHMARK
#endif

class UniqueRanges : public QObject
{
  Q_OBJECT

private:
  static std::vector<std::size_t> unique_stl(std::vector<std::size_t> data);

private slots:
  void unique_data();
  void unique();

  // Benchmarks
private:
  void data();
  static std::vector<std::size_t> create(std::size_t size, double repeats);
private slots:
  void baseline_data();
  void baseline();

  void benchmark_data();
  void benchmark();
};

std::vector<std::size_t> UniqueRanges::unique_stl(std::vector<std::size_t> data)
{
  const auto ret = std::ranges::unique(data);
  data.erase(ret.begin(), ret.end());
  return data;
}

void UniqueRanges::unique_data()
{
  QTest::addColumn<std::vector<std::size_t>>("input");

  QTest::newRow("empty") << std::vector<std::size_t>{};
  QTest::newRow("single") << std::vector<std::size_t>{42};
  QTest::newRow("none") << std::vector<std::size_t>{1, 2, 3, 4, 5, 6, 7};
  QTest::newRow("leading") << std::vector<std::size_t>{/**/ 1, 1, 1 /**/, 2, 3, 5};
  QTest::newRow("middle") << std::vector<std::size_t>{1, 2, /**/ 4, 4, 4 /**/, 8, 16};
  QTest::newRow("trailing") << std::vector<std::size_t>{1, 2, 4, /**/ 8, 8, 8, 8 /**/};
  QTest::newRow("full") << std::vector<std::size_t>{/**/ 1, 1, 1, 1, 1 /**/};
  QTest::newRow("entirerepeats") << std::vector<std::size_t>{/**/ 1, 1, 1, 1, 1, 1, 1, 1 /**/};
}

void UniqueRanges::unique()
{
  const QFETCH(std::vector<std::size_t>, input);

  const std::vector<std::size_t> reference = unique_stl(input);

  const UniqueView u(input);
  QCOMPARE(u.size(), reference.size());
  if (reference.empty())
  {
    QVERIFY(u.begin() == u.end());
  }

  if (!std::equal(u.begin(), u.end(), reference.begin()))
  {
    qDebug() << "Different elements?";
    for (const auto &element : u)
    {
      qDebug() << element;
    }
  }

  QVERIFY(std::equal(u.begin(), u.end(), reference.begin()));
}

// Benchmark

std::vector<std::size_t> UniqueRanges::create(std::size_t size, double repeats)
{
  std::vector<std::size_t> input(size);
  std::iota(input.begin(), input.end(), 0u);
  if (repeats > 0.0)
  {
    const double multiplier = 1.0 - repeats;
    for (std::size_t &element : input)
    {
      element = static_cast<std::size_t>(element * multiplier);
    }
  }
  return input;
}

void UniqueRanges::data()
{
#ifdef QT_DEBUG
  const std::vector<std::size_t> sizes{100000};
#else
  const std::vector<std::size_t> sizes{10000, 100000, 1000000, 10000000, 100000000};
#endif
  const std::vector<double> repeats{0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 1.0};

  QTest::addColumn<std::size_t>("size");
  QTest::addColumn<double>("repeats");

  for (const std::size_t &size : sizes)
  {
    for (const double &repeats : repeats)
    {
      const auto tag = QString("N=%1, r=%2").arg(size).arg(repeats);
      QTest::newRow(tag.toStdString().c_str()) << size << repeats;
    }
  }
}

void UniqueRanges::baseline_data() { data(); }
void UniqueRanges::baseline()
{
  const QFETCH(std::size_t, size);
  const QFETCH(double, repeats);

  const std::vector<std::size_t> input = create(size, repeats);

  MYBENCHMARK
  {
    const auto work = unique_stl(input);
    const auto sum  = std::accumulate(work.begin(), work.end(), 0);
    Q_UNUSED(sum)
  }
}

void UniqueRanges::benchmark_data() { data(); }
void UniqueRanges::benchmark()
{
  const QFETCH(std::size_t, size);
  const QFETCH(double, repeats);

  const std::vector<std::size_t> input = create(size, repeats);

  // Sanity check
  {
    const std::vector<std::size_t> reference = unique_stl(input);

    const UniqueView u(input);
    QCOMPARE(u.size(), reference.size());
    QVERIFY(std::equal(u.begin(), u.end(), reference.begin()));
  }

  // Time
  MYBENCHMARK
  {
    const UniqueView u(input);
    const auto sum = std::accumulate(u.begin(), u.end(), 0);
    Q_UNUSED(sum)
  }
}

QTEST_APPLESS_MAIN(UniqueRanges)

#include "tst_uniqueranges.moc"
