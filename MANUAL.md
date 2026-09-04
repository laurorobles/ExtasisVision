# 📖 EXTASIS VISION v2.0 - MANUAL DEL USUARIO

Bienvenido a **Extasis Vision**, un sintetizador aditivo y sustractivo que lee imágenes en tiempo real.

## 1. Instalación y Licencia
Al abrir el plugin por primera vez, la pantalla estará bloqueada pidiendo una licencia.
Introduce tu clave con formato `EXTV-XXXX-XXXX-XXXX-XXXX` adquirida en Gumroad y presiona **ACTIVATE LICENSE**.

## 2. Flujo de Trabajo (Drag & Drop)
Para comenzar a generar sonido, arrastra cualquier imagen (JPG, PNG) directamente a la pantalla del plugin. El motor analizará la imagen y comenzará a generar notas musicales.

## 3. Controles Superiores (Motores)
- **Engine Mode**: 
  - *Analytic Scanner*: Toca melodías precisas buscando el píxel más brillante.
  - *RGB Synth*: Toca acordes ambientales promediando colores (Verde=Pitch, Azul=Volumen, Rojo=Filtro).
- **Scale Mode**: Obliga al sintetizador a afinar las frecuencias a una escala específica (Cromática o Pentatónica Menor).
- **Sync Mode (1 & 2)**: Conecta el escáner al Tempo de tu canción. Si eliges `1/4`, el escáner cruzará la imagen en exactamente 1 negra (beat).

## 4. Módulos de Escaneo (Inferior)
- **SPEED**: Velocidad en Hz (si Sync está en Free Hz) de los láseres Rojo y Cian.
- **OCTAVE**: Transposición de octavas. Usa el Escáner 1 para bajos (-2) y el Escáner 2 para melodías altas (+1).
- **ECHO MIX**: Cantidad de delay rítmico.
- **SPACE MIX**: Tamaño de la cámara de reverberación espacial.
