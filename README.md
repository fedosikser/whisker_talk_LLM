# ASCII Cat Yard Simulation

Живая ASCII-симуляция, где маленькие котики бегают по полю, останавливаются, общаются друг с другом и показывают реплики прямо на карте.

Проект сочетает:

- уютную консольную симуляцию с персонажами, ролями и движением;
- локальную LLM на `127.0.0.1:1234` для коротких живых диалогов между котиками.

## Сборка

```bash
g++  main.cpp -o main
```

## Запуск

```bash
./main
```

При запуске программа спросит размер окна:

- `Введите высоту окна [64]:`
- `Введите ширину окна [207]:`

Если просто нажать `Enter`, будут использованы значения по умолчанию:

- высота `64`
- ширина `207`

Минимальные размеры:

- высота `20`
- ширина `70`

## Что делает программа

После запуска создаётся поле заданного размера. На нём живут несколько котиков, и каждый из них имеет:

- имя;
- роль;
- характер;
- маленький ASCII-спрайт;
- собственное движение;
- текущую реплику в плашке.

Котики:

- гуляют по полю;
- иногда останавливаются;
- могут заговорить с ближайшим котиком;
- показывают реплики прямо на поле рядом с собой.

Плашки реплик стараются не накладываться друг на друга.

## Диалоги через LLM

Файл: [llm_client.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/llm_client.h)

`LLMClient` настроен на локальный сервер:

- host: `127.0.0.1`
- port: `1234`
- endpoint: `/v1/chat/completions`

Когда два котика начинают разговор, программа формирует короткий prompt и просит локальную модель вернуть диалог в таком формате:

```text
Имя1: реплика
Имя2: ответ
```

Если локальная модель недоступна или отвечает не в нужном формате, симуляция не падает: используются встроенные fallback-реплики.

Логика диалога находится в [cat_simulation.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/cat_simulation.h).

## Почему теперь не съезжает текст

Раньше русские плашки ломали ASCII-сетку, потому что поле хранилось в `char`.

Сейчас:

- матрица поля хранится в wide-символах в [matrix.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/matrix.h);
- UTF-8 строки конвертируются для вывода через [utf_utils.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/utf_utils.h);
- плашки рисуются через [field_bubbles.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/field_bubbles.h).

## Где менять настройки

### Скорость симуляции

Файл: [main.cpp](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/main.cpp)

Ищи строку:

```cpp
sleep_ms(700);
```

Больше число — медленнее движение и обновление.

### Имена, роли, характеры и спрайты котиков

Файл: [cat_simulation.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/cat_simulation.h)

Функция:

```cpp
createCats(...)
```

Там задаются:

- имена котиков;
- роли;
- черты характера;
- ASCII-спрайты;
- стартовые позиции;
- направления движения.

### Когда котики разговаривают

Файл: [cat_simulation.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/cat_simulation.h)

Основная логика:

- `moveCats(...)`
- `updateConversation(...)`
- `startDialogue(...)`

Там можно менять:

- как часто котики останавливаются;
- на каком расстоянии они начинают локально взаимодействовать;
- сколько тактов держится реплика.

### Плашки реплик

Файл: [field_bubbles.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/field_bubbles.h)

Там находится:

- перенос текста;
- размеры плашек;
- выбор позиции плашки рядом с котиком;
- защита от наложения плашек друг на друга.

## Файлы проекта

- [main.cpp](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/main.cpp) — запуск симуляции и главный цикл
- [cat_simulation.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/cat_simulation.h) — котики, движение и диалоги
- [field_bubbles.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/field_bubbles.h) — плашки реплик на поле
- [llm_client.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/llm_client.h) — клиент локальной LLM
- [matrix.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/matrix.h) — матрица символов поля
- [utf_utils.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/utf_utils.h) — UTF-8/Wide конвертация
- [utils.h](/Users/dmitrijkuznecov/c++/Grokaem/graph_designer/utils.h) — очистка экрана и задержки

## Ограничения

- Для живых диалогов нужен локальный LLM-сервер на `127.0.0.1:1234`.
- Если сервер не запущен, котики всё равно продолжают симуляцию, но говорят fallback-репликами.
- В `utf_utils.h` используются deprecated-конвертеры стандартной библиотеки; проект собирается, но компилятор может показывать предупреждения.
