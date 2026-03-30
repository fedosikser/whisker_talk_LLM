# Graph Designer

Небольшой консольный ASCII-проект на C++.

Сейчас в проекте есть две основные части:

- генерация ASCII-барсуков на поле;
- `LLMClient` для подключения к локальной LLM на `127.0.0.1:1234`.

## Сборка

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o main
```

## Запуск

```bash
./main
```

При старте программа спросит размер окна:

- `Введите высоту окна [64]:`
- `Введите ширину окна [207]:`

Если просто нажать `Enter`, будут использованы значения по умолчанию:

- высота `64`
- ширина `207`

## Что делает программа

После запуска создаётся поле заданного размера, и на нём постепенно появляются барсуки в разных ASCII-вариантах.

Для каждого нового барсука:

- случайно выбирается один из вариантов из [badger_variants.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/badger_variants.h);
- выбирается случайная позиция;
- проверяется, что новый барсук не пересекается с уже существующими;
- дополнительно соблюдается минимальная дистанция между барсуками.

## Где менять настройки

### Минимальное расстояние между барсуками

Файл: [main.cpp](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/main.cpp)

Функция: `CreareArt(Point point)`

Строка настройки:

```cpp
const int spacing = 5;
```

Увеличь число, если хочешь больше расстояние между барсуками.

### Варианты барсуков

Файл: [badger_variants.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/badger_variants.h)

Там хранится список ASCII-вариантов, из которых случайно выбирается новый барсук.

### Дефолтный размер окна

Файл: [main.cpp](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/main.cpp)

Сейчас дефолты такие:

- высота `64`
- ширина `207`

## LLM client

Файл: [llm_client.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/llm_client.h)

`LLMClient` настроен на локальный сервер:

- host: `127.0.0.1`
- port: `1234`
- endpoint: `/v1/chat/completions`

Он отправляет HTTP-запросы к локальной LLM и ожидает OpenAI-совместимый формат ответа.

## Файлы проекта

- [main.cpp](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/main.cpp) — основной запуск и генерация поля
- [badger_variants.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/badger_variants.h) — ASCII-варианты барсуков
- [llm_client.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/llm_client.h) — клиент локальной LLM
- [matrix.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/matrix.h) — работа с матрицей символов
- [utils.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/utils.h) — очистка экрана и задержки
- [ascii_art.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/ascii_art.h) — вспомогательная работа с ASCII-art
