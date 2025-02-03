﻿(Español)
/////////////////////////////////////////////////////
RisohEditor por katahiromz
/////////////////////////////////////////////////////

RisohEditor es un editor de recursos gratuito para el desarrollo de Win32.
Puede editar, extraer, clonar y eliminar los datos de recursos en 
archivos RC/RES/EXE/DLL.

Descargar binario: https://katahiromz.web.fc2.com/re/en

Funciona en Windows XP/2003/Vista/7/8.1/10 y ReactOS.

Consulte "LICENSE.txt" para obtener detalles sobre los derechos de autor 
y el acuerdo de licencia.

/////////////////////////////////////////////////////

Pregunta 1. Qué es "Risoh"?

 Respuesta. La palabra "Risoh" significa "ideal" en Japonés.

Pregunta 2. Qué son edt1, edt2, cmb1?

 Respuesta. Esas son macros de ID de control estándar definidas en <dlgs.h>.

Pregunta 3. Qué es mcdx?

 Respuesta. Es un compilador de mensajes especial que creé.
            Consulte mcdx/MESSAGETABLEDX.md para obtener más detalles.

Pregunta 4. ¿Por qué obtuve caracteres ilegibles al compilar con Visual Studio?

 Respuesta. El compilador de recursos de MSVC tiene un error en el tratamiento
            de archivos de recursos UTF-8.

            Use UTF-16 (pero UTF-16 no está soportado en GNU windres).

Pregunta 5. ¿Cuál es la diferencia entre la versión sin instalador y la versión portable?

 Respuesta. La versión portable no utiliza el registro sino un archivo ini.

Pregunta 6. ¿Están soportados los archivos de 64 bits?

 Respuesta. Sí, en Windows de 64 bits. Sin embargo, la capa de emulación de WoW64
            impide que se cargue desde "C:\Archivos de programa" o "C:\Windows\system32".
            Usted debe copiar el archivo de 64 bits en otro lugar antes de cargarlo.

/////////////////////////////////////////////////////////////////////
Katayama Hirofumi MZ (katahiromz) [A.N.T.]
Página Web (Inglés):    https://katahiromz.web.fc2.com/re/en
Página Web (Chino):    https://katahiromz.web.fc2.com/re/ch
Página Web (Japonés):   https://katahiromz.web.fc2.com/re/ja
Página Web (Italiano):    https://katahiromz.web.fc2.com/re/it
Página Web (Ruso):    https://katahiromz.web.fc2.com/re/ru
Página Web (Portugués): https://katahiromz.web.fc2.com/re/pt
Email                 katayama.hirofumi.mz@gmail.com
/////////////////////////////////////////////////////////////////////
