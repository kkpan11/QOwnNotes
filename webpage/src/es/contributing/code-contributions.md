# Contribuciones de código

Gracias por contribuir código a QOwnNotes.

## Solicitudes de incorporación

### Abrir una incidencia primero

Por favor, abra una incidencia antes de abrir una solicitud de incorporación.

Esto ayuda a debatir el problema, la solución propuesta, el alcance y los posibles efectos secundarios antes de que se revise el código.

La única excepción es algo obviamente trivial, como una errata, una pequeña corrección en la documentación o cualquier otro cambio mínimo que no requiera discusión previa.

Si ya existe una incidencia relacionada con su trabajo, haga referencia a ella en su solicitud de incorporación.

### Ramas y solicitudes de incorporación

Por favor, abre solicitudes de incorporación hacia la rama `main`.

Antes de comenzar su trabajo, cree una nueva rama para su cambio y abra la solicitud de incorporación desde esa rama.

Por favor, no abra solicitudes de extracción desde su rama principal, ya que esto puede causar problemas para mantener su bifurcación actualizada y puede provocar la inclusión accidental de cambios no relacionados en la solicitud de incorporación.

### Contribuciones de traducción

Por favor, no realice contribuciones de traducción modificando directamente los archivos de traducción.
Los archivos de traducción se generan automáticamente, por lo que el trabajo de traducción debe realizarse a través de la página de traducción (translation.md).

## Mensajes de confirmación

Por favor, siga el estilo de mensaje de confirmación que se utiliza en este repositorio.

Ejemplos recientes:

- `#1765 misc: optionally anonymize personal information in debug output`
- `#1789 misc: add remembered file manager open prompts`
- `#3568 mainwindow: allow dragging text file to import as note`
- `ci: fix step name`
- `release: bump version`

En general, los mensajes de confirmación deberían:

- Ser cortos y descriptivos
- Usar un ámbito seguido de dos puntos, como `misc:`, `mainwindow:`, `tests:` o `ci:`
- Usar una descripción imperativa después de los dos puntos, como `fix`, `add`, `allow`, `update` o `refactor`
- Incluir el número de incidencia al principio cuando lo haya, por ejemplo: `#3568 mainwindow: allow dragging text file to import as note`

Por favor, mantén las confirmaciones centradas, de modo que cada confirmación represente un cambio lógico.
