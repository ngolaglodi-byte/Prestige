# Prestige AI — Beta 1.0

## Installation rapide

1. Ouvre **PrestigeAI-1.0-beta-macOS.dmg**
2. Glisse **Prestige AI** dans **Applications**
3. Clic droit sur l'app > **Ouvrir** (premier lancement uniquement, contourne Gatekeeper)
4. Autorise l'acces camera quand macOS le demande

## Premier lancement

Les dependances Python s'installent automatiquement au premier demarrage (~2 minutes, connexion internet requise). Une notification macOS t'informe de la progression.

## Ce que fait la beta

- Detection de visages via webcam en temps reel
- Overlay nom/role sur les talents detectes
- Interface Control Room complete (preview, overlays, parametres)
- Communication temps reel Python IA <-> C++ Vision Engine via ZeroMQ
- Export NDI possible (si NDI Tools installe)

## Architecture

L'app lance automatiquement 3 composants :

| Composant | Role |
|---|---|
| **AI Engine** (Python) | Detection et reconnaissance faciale via InsightFace/OpenCV |
| **Vision Engine** (C++) | Compositing GPU des overlays sur le flux video |
| **Control Room** (C++) | Interface operateur avec preview temps reel |

## Configuration

Fichier de config : `~/.prestige-ai/config.json`

## Logs

Tous les logs sont dans : `~/Library/Logs/PrestigeAI/`

- `launch.log` — Log principal du launcher
- `ai_engine.log` — Sortie du moteur IA Python
- `vision.log` — Sortie du Vision Engine C++

## Configuration requise

- macOS 13.0+ (Ventura ou plus recent)
- Apple Silicon (M1/M2/M3/M4) ou Intel
- Python 3.10+ (installe avec `brew install python@3.12`)
- Webcam (integree ou externe)
- 1 GB d'espace disque libre

## Problemes connus

- **Premiere installation Python** : ~2 min d'attente, c'est normal
- **Apple Silicon** : InsightFace fonctionne en mode CPU (pas de GPU CUDA)
- **Camera externe** : relancer l'app si elle n'est pas detectee au demarrage
- **Gatekeeper** : Au premier lancement, clic droit > Ouvrir est necessaire car l'app n'est pas signee

## Feedback

Merci de reporter tout bug ou suggestion a : feedback@prestige-ai.com
