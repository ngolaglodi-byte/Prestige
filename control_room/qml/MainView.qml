import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Effects

ApplicationWindow {
    id: window
    visible: true
    width: 1440
    height: 900
    minimumWidth: 1024
    minimumHeight: 700
    title: {
        var name = configManager.channelName
        if (name === "") name = "PRESTIGE AI"
        if (mainWindow.overlaysActive) return name + " — " + window.t("overlays_active")
        return name + " — " + window.t("passthrough")
    }
    color: window.darkMode ? "#07070A" : "#F0F0F4"

    // ── RTL support (Feature 13) ──────────────────────────
    LayoutMirroring.enabled: mainWindow.language === "ar"
    LayoutMirroring.childrenInherit: true

    // ── i18n (Feature 1) ─────────────────────────────────
    property string lang: mainWindow.language

    property var translations: ({
        "fr": { "file": "Fichier", "emission": "\u00C9mission", "style": "Style", "display": "Affichage", "tools": "Outils", "help": "Aide", "language": "Langue", "new_profile": "Nouveau profil...", "save_profile": "Sauvegarder le profil", "export_profile": "Exporter le profil...", "import_profile": "Importer un profil...", "scan_hw": "Scanner le mat\u00E9riel", "refresh_talents": "Actualiser les talents", "quit": "Quitter", "launch_stop": "Lancer / Arr\u00EAter", "fullscreen": "Plein \u00E9cran", "theme": "Th\u00E8me clair / sombre", "setup_mode": "Mode Setup", "live_mode": "Mode Live", "talent_mgmt": "Gestion des talents...", "sdk_status": "Statut des SDKs...", "teleprompter": "T\u00E9l\u00E9prompteur...", "macros": "Macros & S\u00E9quences...", "ticker": "Ticker / Bandeau...", "stats": "Statistiques \u00E9mission...", "shortcuts": "Raccourcis clavier", "about": "\u00C0 propos de Prestige AI", "remote": "Contr\u00F4le distant...", "all_styles": "Tous les styles...",
            "on_air": "EN DIRECT", "bypass": "BYPASS", "overlays_on": "OVERLAYS ON", "overlays_off": "OVERLAYS OFF", "stop": "Arr\u00EAter", "stop_emission": "Arr\u00EAter l'\u00E9mission", "launch_emission": "Lancer l'\u00E9mission", "waiting": "En attente de d\u00E9tection...", "detection": "D\u00E9tection", "face": "visage", "faces": "visages", "output_live": "Output live", "launch_vision": "Lancez le Vision Engine pour voir le flux",
            "next": "Suivant", "previous": "Pr\u00E9c\u00E9dent", "save": "Sauvegarder", "launch": "LANCER", "launch_emission_btn": "LANCER L'\u00C9MISSION",
            "step_style": "Choix du style", "step_talents": "Gestion des talents", "step_source": "Source vid\u00E9o", "step_outputs": "Sorties", "step_test": "Test & validation",
            "config_emission": "Configuration \u00E9mission", "active_profile": "Profil actif", "modify": "Modifier...",
            "overlay_style": "Style d'overlay", "hover_preview": "Survolez pour pr\u00E9visualiser", "preview": "Pr\u00E9visualisation", "replay_anim": "Rejouer l'animation", "accent_color": "Couleur accent", "opacity": "Opacit\u00E9", "animation": "Animation", "multi_face": "Multi-visage", "delays": "D\u00E9lais", "enter": "Entr\u00E9e", "exit": "Sortie", "single_face": "1 visage", "multi": "Multi", "hide": "Masquer",
            "emission_talents": "Talents de l'\u00E9mission", "new_talent": "+ Nouveau talent", "refresh": "Actualiser", "ai_connected": "Serveur IA connect\u00E9", "ai_disconnected": "Serveur IA d\u00E9connect\u00E9 \u2014 lancez talent_manager.py", "talents_count": "talents", "no_embedding": "Pas d'embedding", "embedding_count": "embedding(s)", "photo": "Photo", "delete": "Supprimer", "no_talents": "Aucun talent enregistr\u00E9\nCliquez '+' pour en ajouter", "talent_register": "Enregistrement talent", "full_name": "Nom complet", "role_function": "R\u00F4le / Fonction", "cancel": "Annuler", "register": "Enregistrer", "create": "Cr\u00E9er",
            "video_source": "Source vid\u00E9o d'entr\u00E9e", "choose_source": "Choisissez la source vid\u00E9o qui sera analys\u00E9e par l'IA.", "ndi_sources": "Sources NDI disponibles", "search": "Rechercher", "searching_ndi": "Recherche de sources NDI sur le r\u00E9seau...", "connect": "Connecter", "connected": "Connect\u00E9", "no_ndi": "Aucune source NDI d\u00E9tect\u00E9e\nV\u00E9rifiez que vos appareils NDI sont allum\u00E9s", "resolution": "R\u00E9solution",
            "broadcast_outputs": "Sorties broadcast (TV)", "broadcast_desc": "Sorties professionnelles pour la diffusion t\u00E9l\u00E9vis\u00E9e.", "social_outputs": "R\u00E9seaux sociaux (streaming simultan\u00E9)", "social_desc": "Diffusez en direct sur plusieurs plateformes en m\u00EAme temps.", "stream_key": "Cl\u00E9 de stream", "key_configured": "Cl\u00E9 configur\u00E9e", "not_configured": "Non configur\u00E9", "output_summary": "R\u00E9sum\u00E9 des sorties actives", "no_output": "Aucune sortie configur\u00E9e", "social_networks": "r\u00E9seau(x) social(aux)",
            "live_test": "Test en direct", "click_test": "Cliquez pour lancer le test", "launch_test": "Lancer le test (webcam)", "pre_checklist": "Checklist pr\u00E9-\u00E9mission", "style_chosen": "Style d'overlay choisi", "talent_added": "Au moins 1 talent ajout\u00E9", "source_configured": "Source vid\u00E9o configur\u00E9e", "output_active": "Au moins 1 sortie active", "ai_server": "Serveur IA connect\u00E9", "ready": "Pr\u00EAt \u00E0 diffuser", "not_ready": "Configuration incompl\u00E8te", "can_launch": "Vous pouvez lancer l'\u00E9mission", "complete_steps": "Compl\u00E9tez les \u00E9tapes ci-dessus",
            "launch_question": "Lancer l'\u00E9mission ?", "live_switch": "L'interface passera en mode live.\nTout est pr\u00EAt.", "stop_question": "Arr\u00EAter l'\u00E9mission ?", "stop_desc": "Les overlays seront d\u00E9sactiv\u00E9s et toutes\nles sorties seront coup\u00E9es.", "continue": "Continuer",
            "new_profile_title": "Nouveau profil", "emission_name": "Nom de l'\u00E9mission", "profile_saved": "Profil sauvegard\u00E9", "hw_scan_done": "Scan mat\u00E9riel termin\u00E9",
            "sdk_broadcast_status": "Statut des SDKs broadcast", "installed": "Install\u00E9", "close": "Fermer",
            "shortcuts_title": "Raccourcis clavier", "about_desc": "Logiciel broadcast professionnel\navec reconnaissance faciale IA",
            "channel_name_menu": "Nom de la cha\u00EEne...", "channel_name_title": "Nom de la cha\u00EEne", "channel_name_desc": "Ce nom appara\u00EEt sur tous vos overlays en direct", "branding": "Habillage cha\u00EEne...", "branding_title": "Habillage cha\u00EEne",
            "channel_logo": "Logo de la cha\u00EEne", "browse": "Parcourir...", "position": "Position", "size": "Taille", "entry_anim": "Animation entr\u00E9e", "loop_anim": "Animation boucle", "keep_logo_ads": "Garder le logo pendant les pubs",
            "channel_name_section": "Nom de la cha\u00EEne", "show_name_text": "Afficher le nom en texte", "shape": "Forme", "bg_color": "Fond", "text_color": "Texte", "border_color": "Bordure", "font_size": "Taille police",
            "show_title_section": "Titre de l'emission", "enable_title": "Activer le titre permanent", "title": "Titre", "subtitle": "Sous-titre",
            "show_title_shape": "Forme", "show_title_bg": "Fond", "show_title_text": "Texte", "show_title_border": "Bordure",
            "cycle_talent_title": "Cycle talent / titre", "talent_display_dur": "Dur\u00E9e affichage talent", "title_reappear": "D\u00E9lai retour titre",
            "welcome": "Bienvenue", "enter_channel": "Entrez le nom de votre cha\u00EEne", "channel_overlay_desc": "Ce nom appara\u00EEtra sur tous vos overlays en direct", "start": "COMMENCER",
            "export_title": "Exporter le profil", "file_path": "Chemin du fichier", "export": "Exporter", "import_title": "Importer un profil", "json_path": "Chemin du fichier JSON", "import": "Importer",
            "choose_logo": "Choisir un logo", "source_config": "Configuration source...", "output_config": "Configuration sorties...",
            "top_right": "Haut droite", "top_left": "Haut gauche", "bottom_right": "Bas droite", "bottom_left": "Bas gauche",
            "rec": "REC", "scoreboard": "Score", "weather": "M\u00E9t\u00E9o", "subtitles": "Sous-titres", "social": "Social",
            "overlays_active": "OVERLAYS ACTIFS", "passthrough": "PASSTHROUGH", "programs": "PROGRAMMES", "current_program": "EN COURS", "new_program": "Nouveau programme", "configure": "Configurer...", "activate_overlays": "Activer les overlays", "deactivate_overlays": "D\u00E9sactiver les overlays", "activate_overlays_desc": "Les overlays seront activ\u00E9s.\nLe signal continue en permanence.", "deactivate_overlays_desc": "Les overlays seront d\u00E9sactiv\u00E9s.\nLe signal continue en passthrough.", "next_program": "Programme suivant"
        },
        "en": { "file": "File", "emission": "Broadcast", "style": "Style", "display": "Display", "tools": "Tools", "help": "Help", "language": "Language", "new_profile": "New profile...", "save_profile": "Save profile", "export_profile": "Export profile...", "import_profile": "Import profile...", "scan_hw": "Scan hardware", "refresh_talents": "Refresh talents", "quit": "Quit", "launch_stop": "Launch / Stop", "fullscreen": "Fullscreen", "theme": "Light / dark theme", "setup_mode": "Setup mode", "live_mode": "Live mode", "talent_mgmt": "Talent management...", "sdk_status": "SDK status...", "teleprompter": "Teleprompter...", "macros": "Macros & Sequences...", "ticker": "Ticker / Banner...", "stats": "Broadcast stats...", "shortcuts": "Keyboard shortcuts", "about": "About Prestige AI", "remote": "Remote control...", "all_styles": "All styles...",
            "on_air": "ON AIR", "bypass": "BYPASS", "overlays_on": "OVERLAYS ON", "overlays_off": "OVERLAYS OFF", "stop": "Stop", "stop_emission": "Stop broadcast", "launch_emission": "Launch broadcast", "waiting": "Waiting for detection...", "detection": "Detection", "face": "face", "faces": "faces", "output_live": "Live output", "launch_vision": "Start Vision Engine to see the feed",
            "next": "Next", "previous": "Previous", "save": "Save", "launch": "LAUNCH", "launch_emission_btn": "LAUNCH BROADCAST",
            "step_style": "Style selection", "step_talents": "Talent management", "step_source": "Video source", "step_outputs": "Outputs", "step_test": "Test & validation",
            "config_emission": "Broadcast configuration", "active_profile": "Active profile", "modify": "Modify...",
            "overlay_style": "Overlay style", "hover_preview": "Hover to preview", "preview": "Preview", "replay_anim": "Replay animation", "accent_color": "Accent color", "opacity": "Opacity", "animation": "Animation", "multi_face": "Multi-face", "delays": "Delays", "enter": "Enter", "exit": "Exit", "single_face": "1 face", "multi": "Multi", "hide": "Hide",
            "emission_talents": "Broadcast talents", "new_talent": "+ New talent", "refresh": "Refresh", "ai_connected": "AI server connected", "ai_disconnected": "AI server disconnected \u2014 run talent_manager.py", "talents_count": "talents", "no_embedding": "No embedding", "embedding_count": "embedding(s)", "photo": "Photo", "delete": "Delete", "no_talents": "No talents registered\nClick '+' to add one", "talent_register": "Talent registration", "full_name": "Full name", "role_function": "Role / Title", "cancel": "Cancel", "register": "Save", "create": "Create",
            "video_source": "Video source input", "choose_source": "Choose the video source to be analyzed by AI.", "ndi_sources": "Available NDI sources", "search": "Search", "searching_ndi": "Searching for NDI sources on the network...", "connect": "Connect", "connected": "Connected", "no_ndi": "No NDI source detected\nCheck that your NDI devices are powered on", "resolution": "Resolution",
            "broadcast_outputs": "Broadcast outputs (TV)", "broadcast_desc": "Professional outputs for television broadcast.", "social_outputs": "Social media (simultaneous streaming)", "social_desc": "Stream live to multiple platforms at the same time.", "stream_key": "Stream key", "key_configured": "Key configured", "not_configured": "Not configured", "output_summary": "Active output summary", "no_output": "No output configured", "social_networks": "social network(s)",
            "live_test": "Live test", "click_test": "Click to start the test", "launch_test": "Start test (webcam)", "pre_checklist": "Pre-broadcast checklist", "style_chosen": "Overlay style selected", "talent_added": "At least 1 talent added", "source_configured": "Video source configured", "output_active": "At least 1 output active", "ai_server": "AI server connected", "ready": "Ready to broadcast", "not_ready": "Incomplete configuration", "can_launch": "You can launch the broadcast", "complete_steps": "Complete the steps above",
            "launch_question": "Launch broadcast?", "live_switch": "The interface will switch to live mode.\nEverything is ready.", "stop_question": "Stop broadcast?", "stop_desc": "Overlays will be disabled and all\noutputs will be cut.", "continue": "Continue",
            "new_profile_title": "New profile", "emission_name": "Broadcast name", "profile_saved": "Profile saved", "hw_scan_done": "Hardware scan complete",
            "sdk_broadcast_status": "Broadcast SDK status", "installed": "Installed", "close": "Close",
            "shortcuts_title": "Keyboard shortcuts", "about_desc": "Professional broadcast software\nwith AI facial recognition",
            "channel_name_menu": "Channel name...", "channel_name_title": "Channel name", "channel_name_desc": "This name appears on all your live overlays", "branding": "Channel branding...", "branding_title": "Channel branding",
            "channel_logo": "Channel logo", "browse": "Browse...", "position": "Position", "size": "Size", "entry_anim": "Entry animation", "loop_anim": "Loop animation", "keep_logo_ads": "Keep logo during ads",
            "channel_name_section": "Channel name", "show_name_text": "Show name as text", "shape": "Shape", "bg_color": "Background", "text_color": "Text", "border_color": "Border", "font_size": "Font size",
            "show_title_section": "Show title", "enable_title": "Enable permanent title", "title": "Title", "subtitle": "Subtitle",
            "show_title_shape": "Shape", "show_title_bg": "Background", "show_title_text": "Text", "show_title_border": "Border",
            "cycle_talent_title": "Talent / title cycle", "talent_display_dur": "Talent display duration", "title_reappear": "Title reappear delay",
            "welcome": "Welcome", "enter_channel": "Enter your channel name", "channel_overlay_desc": "This name will appear on all your live overlays", "start": "START",
            "export_title": "Export profile", "file_path": "File path", "export": "Export", "import_title": "Import profile", "json_path": "JSON file path", "import": "Import",
            "choose_logo": "Choose a logo", "source_config": "Source configuration...", "output_config": "Output configuration...",
            "top_right": "Top right", "top_left": "Top left", "bottom_right": "Bottom right", "bottom_left": "Bottom left",
            "rec": "REC", "scoreboard": "Score", "weather": "Weather", "subtitles": "Subtitles", "social": "Social",
            "overlays_active": "OVERLAYS ACTIVE", "passthrough": "PASSTHROUGH", "programs": "PROGRAMS", "current_program": "CURRENT", "new_program": "New program", "configure": "Configure...", "activate_overlays": "Activate overlays", "deactivate_overlays": "Deactivate overlays", "activate_overlays_desc": "Overlays will be activated.\nThe signal continues at all times.", "deactivate_overlays_desc": "Overlays will be deactivated.\nThe signal continues in passthrough.", "next_program": "Next program"
        },
        "es": { "file": "Archivo", "emission": "Emisi\u00F3n", "style": "Estilo", "display": "Pantalla", "tools": "Herramientas", "help": "Ayuda", "language": "Idioma", "new_profile": "Nuevo perfil...", "save_profile": "Guardar perfil", "export_profile": "Exportar perfil...", "import_profile": "Importar perfil...", "scan_hw": "Escanear hardware", "refresh_talents": "Actualizar talentos", "quit": "Salir", "launch_stop": "Iniciar / Parar", "fullscreen": "Pantalla completa", "theme": "Tema claro / oscuro", "setup_mode": "Modo Config", "live_mode": "Modo Directo", "talent_mgmt": "Gesti\u00F3n de talentos...", "sdk_status": "Estado de SDKs...", "teleprompter": "Teleprompter...", "macros": "Macros & Secuencias...", "ticker": "Ticker / Banner...", "stats": "Estad\u00EDsticas...", "shortcuts": "Atajos de teclado", "about": "Acerca de Prestige AI", "remote": "Control remoto...", "all_styles": "Todos los estilos...",
            "on_air": "EN VIVO", "bypass": "BYPASS", "overlays_on": "OVERLAYS ON", "overlays_off": "OVERLAYS OFF", "stop": "Detener", "stop_emission": "Detener emisi\u00F3n", "launch_emission": "Iniciar emisi\u00F3n", "waiting": "Esperando detecci\u00F3n...", "detection": "Detecci\u00F3n", "face": "rostro", "faces": "rostros", "output_live": "Salida en vivo", "launch_vision": "Inicie Vision Engine para ver el flujo",
            "next": "Siguiente", "previous": "Anterior", "save": "Guardar", "launch": "INICIAR", "launch_emission_btn": "INICIAR EMISI\u00D3N",
            "step_style": "Selecci\u00F3n de estilo", "step_talents": "Gesti\u00F3n de talentos", "step_source": "Fuente de video", "step_outputs": "Salidas", "step_test": "Test & validaci\u00F3n",
            "config_emission": "Configuraci\u00F3n de emisi\u00F3n", "active_profile": "Perfil activo", "modify": "Modificar...",
            "overlay_style": "Estilo de overlay", "hover_preview": "Pase el rat\u00F3n para previsualizar", "preview": "Vista previa", "replay_anim": "Reproducir animaci\u00F3n", "accent_color": "Color de acento", "opacity": "Opacidad", "animation": "Animaci\u00F3n", "multi_face": "Multi-rostro", "delays": "Retardos", "enter": "Entrada", "exit": "Salida", "single_face": "1 rostro", "multi": "Multi", "hide": "Ocultar",
            "emission_talents": "Talentos de la emisi\u00F3n", "new_talent": "+ Nuevo talento", "refresh": "Actualizar", "ai_connected": "Servidor IA conectado", "ai_disconnected": "Servidor IA desconectado \u2014 ejecute talent_manager.py", "talents_count": "talentos", "no_embedding": "Sin embedding", "embedding_count": "embedding(s)", "photo": "Foto", "delete": "Eliminar", "no_talents": "Sin talentos registrados\nHaga clic en '+' para a\u00F1adir", "talent_register": "Registro de talento", "full_name": "Nombre completo", "role_function": "Rol / Funci\u00F3n", "cancel": "Cancelar", "register": "Guardar", "create": "Crear",
            "video_source": "Fuente de video", "choose_source": "Elija la fuente de video que ser\u00E1 analizada por la IA.", "ndi_sources": "Fuentes NDI disponibles", "search": "Buscar", "searching_ndi": "Buscando fuentes NDI en la red...", "connect": "Conectar", "connected": "Conectado", "no_ndi": "Ninguna fuente NDI detectada\nVerifique que sus dispositivos NDI est\u00E9n encendidos", "resolution": "Resoluci\u00F3n",
            "broadcast_outputs": "Salidas broadcast (TV)", "broadcast_desc": "Salidas profesionales para transmisi\u00F3n televisiva.", "social_outputs": "Redes sociales (streaming simult\u00E1neo)", "social_desc": "Transmita en directo en varias plataformas al mismo tiempo.", "stream_key": "Clave de stream", "key_configured": "Clave configurada", "not_configured": "No configurado", "output_summary": "Resumen de salidas activas", "no_output": "Ninguna salida configurada", "social_networks": "red(es) social(es)",
            "live_test": "Test en directo", "click_test": "Haga clic para iniciar el test", "launch_test": "Iniciar test (webcam)", "pre_checklist": "Checklist pre-emisi\u00F3n", "style_chosen": "Estilo de overlay seleccionado", "talent_added": "Al menos 1 talento a\u00F1adido", "source_configured": "Fuente de video configurada", "output_active": "Al menos 1 salida activa", "ai_server": "Servidor IA conectado", "ready": "Listo para transmitir", "not_ready": "Configuraci\u00F3n incompleta", "can_launch": "Puede iniciar la emisi\u00F3n", "complete_steps": "Complete los pasos anteriores",
            "launch_question": "\u00BFIniciar emisi\u00F3n?", "live_switch": "La interfaz cambiar\u00E1 al modo directo.\nTodo est\u00E1 listo.", "stop_question": "\u00BFDetener emisi\u00F3n?", "stop_desc": "Los overlays se desactivar\u00E1n y todas\nlas salidas se cortar\u00E1n.", "continue": "Continuar",
            "new_profile_title": "Nuevo perfil", "emission_name": "Nombre de emisi\u00F3n", "profile_saved": "Perfil guardado", "hw_scan_done": "Escaneo de hardware completado",
            "sdk_broadcast_status": "Estado de SDKs broadcast", "installed": "Instalado", "close": "Cerrar",
            "shortcuts_title": "Atajos de teclado", "about_desc": "Software broadcast profesional\ncon reconocimiento facial IA",
            "channel_name_menu": "Nombre del canal...", "channel_name_title": "Nombre del canal", "channel_name_desc": "Este nombre aparece en todos sus overlays en directo", "branding": "Imagen del canal...", "branding_title": "Imagen del canal",
            "channel_logo": "Logo del canal", "browse": "Examinar...", "position": "Posici\u00F3n", "size": "Tama\u00F1o", "entry_anim": "Animaci\u00F3n de entrada", "loop_anim": "Animaci\u00F3n en bucle", "keep_logo_ads": "Mantener logo durante anuncios",
            "channel_name_section": "Nombre del canal", "show_name_text": "Mostrar nombre como texto", "shape": "Forma", "bg_color": "Fondo", "text_color": "Texto", "border_color": "Borde", "font_size": "Tama\u00F1o de fuente",
            "show_title_section": "T\u00EDtulo del programa", "enable_title": "Activar t\u00EDtulo permanente", "title": "T\u00EDtulo", "subtitle": "Subt\u00EDtulo",
            "show_title_shape": "Forma", "show_title_bg": "Fondo", "show_title_text": "Texto", "show_title_border": "Borde",
            "cycle_talent_title": "Ciclo talento / t\u00EDtulo", "talent_display_dur": "Duraci\u00F3n de visualizaci\u00F3n", "title_reappear": "Retardo reaparici\u00F3n t\u00EDtulo",
            "welcome": "Bienvenido", "enter_channel": "Introduzca el nombre de su canal", "channel_overlay_desc": "Este nombre aparecer\u00E1 en todos sus overlays en directo", "start": "COMENZAR",
            "export_title": "Exportar perfil", "file_path": "Ruta del archivo", "export": "Exportar", "import_title": "Importar perfil", "json_path": "Ruta del archivo JSON", "import": "Importar",
            "choose_logo": "Elegir un logo", "source_config": "Configuraci\u00F3n de fuente...", "output_config": "Configuraci\u00F3n de salidas...",
            "top_right": "Arriba derecha", "top_left": "Arriba izquierda", "bottom_right": "Abajo derecha", "bottom_left": "Abajo izquierda",
            "rec": "REC", "scoreboard": "Marcador", "weather": "Clima", "subtitles": "Subt\u00EDtulos", "social": "Social",
            "overlays_active": "OVERLAYS ACTIVOS", "passthrough": "PASSTHROUGH", "programs": "PROGRAMAS", "current_program": "ACTUAL", "new_program": "Nuevo programa", "configure": "Configurar...", "activate_overlays": "Activar overlays", "deactivate_overlays": "Desactivar overlays", "activate_overlays_desc": "Los overlays se activar\u00E1n.\nLa se\u00F1al contin\u00FAa en todo momento.", "deactivate_overlays_desc": "Los overlays se desactivar\u00E1n.\nLa se\u00F1al contin\u00FAa en passthrough.", "next_program": "Siguiente programa"
        },
        "ar": { "file": "\u0645\u0644\u0641", "emission": "\u0627\u0644\u0628\u062B", "style": "\u0627\u0644\u0646\u0645\u0637", "display": "\u0627\u0644\u0639\u0631\u0636", "tools": "\u0623\u062F\u0648\u0627\u062A", "help": "\u0645\u0633\u0627\u0639\u062F\u0629", "language": "\u0627\u0644\u0644\u063A\u0629", "new_profile": "\u0645\u0644\u0641 \u062C\u062F\u064A\u062F...", "save_profile": "\u062D\u0641\u0638", "export_profile": "\u062A\u0635\u062F\u064A\u0631...", "import_profile": "\u0627\u0633\u062A\u064A\u0631\u0627\u062F...", "scan_hw": "\u0641\u062D\u0635 \u0627\u0644\u0623\u062C\u0647\u0632\u0629", "refresh_talents": "\u062A\u062D\u062F\u064A\u062B", "quit": "\u062E\u0631\u0648\u062C", "launch_stop": "\u0628\u062F\u0621 / \u0625\u064A\u0642\u0627\u0641", "fullscreen": "\u0645\u0644\u0621 \u0627\u0644\u0634\u0627\u0634\u0629", "theme": "\u0627\u0644\u0645\u0638\u0647\u0631", "setup_mode": "\u0648\u0636\u0639 \u0627\u0644\u0625\u0639\u062F\u0627\u062F", "live_mode": "\u0648\u0636\u0639 \u0627\u0644\u0628\u062B", "talent_mgmt": "\u0625\u062F\u0627\u0631\u0629 \u0627\u0644\u0645\u0648\u0627\u0647\u0628...", "sdk_status": "\u062D\u0627\u0644\u0629 SDKs...", "teleprompter": "\u0627\u0644\u0645\u0644\u0642\u0646...", "macros": "\u0645\u0627\u0643\u0631\u0648...", "ticker": "\u0634\u0631\u064A\u0637 \u0627\u0644\u0623\u062E\u0628\u0627\u0631...", "stats": "\u0625\u062D\u0635\u0627\u0626\u064A\u0627\u062A...", "shortcuts": "\u0627\u062E\u062A\u0635\u0627\u0631\u0627\u062A", "about": "\u062D\u0648\u0644 Prestige AI", "remote": "\u0627\u0644\u062A\u062D\u0643\u0645 \u0639\u0646 \u0628\u0639\u062F...", "all_styles": "\u0643\u0644 \u0627\u0644\u0623\u0646\u0645\u0627\u0637...",
            "on_air": "\u0639\u0644\u0649 \u0627\u0644\u0647\u0648\u0627\u0621", "bypass": "BYPASS", "overlays_on": "OVERLAYS ON", "overlays_off": "OVERLAYS OFF", "stop": "\u0625\u064A\u0642\u0627\u0641", "stop_emission": "\u0625\u064A\u0642\u0627\u0641 \u0627\u0644\u0628\u062B", "launch_emission": "\u0628\u062F\u0621 \u0627\u0644\u0628\u062B", "waiting": "\u0628\u0627\u0646\u062A\u0638\u0627\u0631 \u0627\u0644\u0643\u0634\u0641...", "detection": "\u0627\u0644\u0643\u0634\u0641", "face": "\u0648\u062C\u0647", "faces": "\u0648\u062C\u0648\u0647", "output_live": "\u0627\u0644\u0628\u062B \u0627\u0644\u0645\u0628\u0627\u0634\u0631", "launch_vision": "\u0634\u063A\u0644 Vision Engine \u0644\u0631\u0624\u064A\u0629 \u0627\u0644\u0628\u062B",
            "next": "\u0627\u0644\u062A\u0627\u0644\u064A", "previous": "\u0627\u0644\u0633\u0627\u0628\u0642", "save": "\u062D\u0641\u0638", "launch": "\u0628\u062F\u0621", "launch_emission_btn": "\u0628\u062F\u0621 \u0627\u0644\u0628\u062B",
            "step_style": "\u0627\u062E\u062A\u064A\u0627\u0631 \u0627\u0644\u0646\u0645\u0637", "step_talents": "\u0625\u062F\u0627\u0631\u0629 \u0627\u0644\u0645\u0648\u0627\u0647\u0628", "step_source": "\u0645\u0635\u062F\u0631 \u0627\u0644\u0641\u064A\u062F\u064A\u0648", "step_outputs": "\u0627\u0644\u0645\u062E\u0631\u062C\u0627\u062A", "step_test": "\u0627\u062E\u062A\u0628\u0627\u0631",
            "config_emission": "\u0625\u0639\u062F\u0627\u062F \u0627\u0644\u0628\u062B", "active_profile": "\u0627\u0644\u0645\u0644\u0641 \u0627\u0644\u0646\u0634\u0637", "modify": "\u062A\u0639\u062F\u064A\u0644...",
            "overlay_style": "\u0646\u0645\u0637 \u0627\u0644\u0639\u0631\u0636", "hover_preview": "\u0645\u0631\u0631 \u0644\u0644\u0645\u0639\u0627\u064A\u0646\u0629", "preview": "\u0645\u0639\u0627\u064A\u0646\u0629", "replay_anim": "\u0625\u0639\u0627\u062F\u0629 \u0627\u0644\u062A\u0634\u063A\u064A\u0644", "accent_color": "\u0644\u0648\u0646 \u0627\u0644\u062A\u0645\u064A\u064A\u0632", "opacity": "\u0627\u0644\u0634\u0641\u0627\u0641\u064A\u0629", "animation": "\u0627\u0644\u0631\u0633\u0648\u0645 \u0627\u0644\u0645\u062A\u062D\u0631\u0643\u0629", "multi_face": "\u0645\u062A\u0639\u062F\u062F \u0627\u0644\u0648\u062C\u0648\u0647", "delays": "\u0627\u0644\u062A\u0623\u062E\u064A\u0631\u0627\u062A", "enter": "\u062F\u062E\u0648\u0644", "exit": "\u062E\u0631\u0648\u062C", "single_face": "\u0648\u062C\u0647 \u0648\u0627\u062D\u062F", "multi": "\u0645\u062A\u0639\u062F\u062F", "hide": "\u0625\u062E\u0641\u0627\u0621",
            "emission_talents": "\u0645\u0648\u0627\u0647\u0628 \u0627\u0644\u0628\u062B", "new_talent": "+ \u0645\u0648\u0647\u0628\u0629 \u062C\u062F\u064A\u062F\u0629", "refresh": "\u062A\u062D\u062F\u064A\u062B", "ai_connected": "\u062E\u0627\u062F\u0645 \u0627\u0644\u0630\u0643\u0627\u0621 \u0645\u062A\u0635\u0644", "ai_disconnected": "\u062E\u0627\u062F\u0645 \u0627\u0644\u0630\u0643\u0627\u0621 \u063A\u064A\u0631 \u0645\u062A\u0635\u0644", "talents_count": "\u0645\u0648\u0627\u0647\u0628", "no_embedding": "\u0628\u062F\u0648\u0646 embedding", "embedding_count": "embedding(s)", "photo": "\u0635\u0648\u0631\u0629", "delete": "\u062D\u0630\u0641", "no_talents": "\u0644\u0627 \u0645\u0648\u0627\u0647\u0628 \u0645\u0633\u062C\u0644\u0629", "talent_register": "\u062A\u0633\u062C\u064A\u0644 \u0627\u0644\u0645\u0648\u0647\u0628\u0629", "full_name": "\u0627\u0644\u0627\u0633\u0645 \u0627\u0644\u0643\u0627\u0645\u0644", "role_function": "\u0627\u0644\u062F\u0648\u0631", "cancel": "\u0625\u0644\u063A\u0627\u0621", "register": "\u062D\u0641\u0638", "create": "\u0625\u0646\u0634\u0627\u0621",
            "video_source": "\u0645\u0635\u062F\u0631 \u0627\u0644\u0641\u064A\u062F\u064A\u0648", "choose_source": "\u0627\u062E\u062A\u0631 \u0645\u0635\u062F\u0631 \u0627\u0644\u0641\u064A\u062F\u064A\u0648.", "ndi_sources": "\u0645\u0635\u0627\u062F\u0631 NDI \u0627\u0644\u0645\u062A\u0627\u062D\u0629", "search": "\u0628\u062D\u062B", "searching_ndi": "\u062C\u0627\u0631\u064A \u0627\u0644\u0628\u062D\u062B...", "connect": "\u0627\u062A\u0635\u0627\u0644", "connected": "\u0645\u062A\u0635\u0644", "no_ndi": "\u0644\u0627 \u0645\u0635\u0627\u062F\u0631 NDI", "resolution": "\u0627\u0644\u062F\u0642\u0629",
            "broadcast_outputs": "\u0645\u062E\u0631\u062C\u0627\u062A \u0627\u0644\u0628\u062B", "broadcast_desc": "\u0645\u062E\u0631\u062C\u0627\u062A \u0627\u062D\u062A\u0631\u0627\u0641\u064A\u0629.", "social_outputs": "\u0648\u0633\u0627\u0626\u0644 \u0627\u0644\u062A\u0648\u0627\u0635\u0644", "social_desc": "\u0628\u062B \u0645\u0628\u0627\u0634\u0631 \u0639\u0644\u0649 \u0639\u062F\u0629 \u0645\u0646\u0635\u0627\u062A.", "stream_key": "\u0645\u0641\u062A\u0627\u062D \u0627\u0644\u0628\u062B", "key_configured": "\u0645\u0641\u062A\u0627\u062D \u0645\u0647\u064A\u0623", "not_configured": "\u063A\u064A\u0631 \u0645\u0647\u064A\u0623", "output_summary": "\u0645\u0644\u062E\u0635 \u0627\u0644\u0645\u062E\u0631\u062C\u0627\u062A", "no_output": "\u0644\u0627 \u0645\u062E\u0631\u062C\u0627\u062A", "social_networks": "\u0634\u0628\u0643\u0629 \u0627\u062C\u062A\u0645\u0627\u0639\u064A\u0629",
            "live_test": "\u0627\u062E\u062A\u0628\u0627\u0631 \u0645\u0628\u0627\u0634\u0631", "click_test": "\u0627\u0646\u0642\u0631 \u0644\u0628\u062F\u0621 \u0627\u0644\u0627\u062E\u062A\u0628\u0627\u0631", "launch_test": "\u0628\u062F\u0621 \u0627\u0644\u0627\u062E\u062A\u0628\u0627\u0631", "pre_checklist": "\u0642\u0627\u0626\u0645\u0629 \u0627\u0644\u0641\u062D\u0635", "style_chosen": "\u0646\u0645\u0637 \u0645\u062E\u062A\u0627\u0631", "talent_added": "\u0645\u0648\u0647\u0628\u0629 \u0645\u0636\u0627\u0641\u0629", "source_configured": "\u0645\u0635\u062F\u0631 \u0645\u0647\u064A\u0623", "output_active": "\u0645\u062E\u0631\u062C \u0646\u0634\u0637", "ai_server": "\u062E\u0627\u062F\u0645 \u0627\u0644\u0630\u0643\u0627\u0621 \u0645\u062A\u0635\u0644", "ready": "\u062C\u0627\u0647\u0632 \u0644\u0644\u0628\u062B", "not_ready": "\u0625\u0639\u062F\u0627\u062F \u063A\u064A\u0631 \u0645\u0643\u062A\u0645\u0644", "can_launch": "\u064A\u0645\u0643\u0646\u0643 \u0628\u062F\u0621 \u0627\u0644\u0628\u062B", "complete_steps": "\u0623\u0643\u0645\u0644 \u0627\u0644\u062E\u0637\u0648\u0627\u062A",
            "launch_question": "\u0628\u062F\u0621 \u0627\u0644\u0628\u062B\u061F", "live_switch": "\u0633\u062A\u062A\u062D\u0648\u0644 \u0627\u0644\u0648\u0627\u062C\u0647\u0629 \u0625\u0644\u0649 \u0627\u0644\u0628\u062B \u0627\u0644\u0645\u0628\u0627\u0634\u0631.", "stop_question": "\u0625\u064A\u0642\u0627\u0641 \u0627\u0644\u0628\u062B\u061F", "stop_desc": "\u0633\u064A\u062A\u0645 \u062A\u0639\u0637\u064A\u0644 \u0643\u0644 \u0634\u064A\u0621.", "continue": "\u0645\u062A\u0627\u0628\u0639\u0629",
            "new_profile_title": "\u0645\u0644\u0641 \u062C\u062F\u064A\u062F", "emission_name": "\u0627\u0633\u0645 \u0627\u0644\u0628\u062B", "profile_saved": "\u062A\u0645 \u0627\u0644\u062D\u0641\u0638", "hw_scan_done": "\u0627\u0643\u062A\u0645\u0644 \u0627\u0644\u0641\u062D\u0635",
            "sdk_broadcast_status": "\u062D\u0627\u0644\u0629 SDKs", "installed": "\u0645\u062B\u0628\u062A", "close": "\u0625\u063A\u0644\u0627\u0642",
            "shortcuts_title": "\u0627\u062E\u062A\u0635\u0627\u0631\u0627\u062A \u0627\u0644\u0644\u0648\u062D\u0629", "about_desc": "\u0628\u0631\u0646\u0627\u0645\u062C \u0628\u062B \u0627\u062D\u062A\u0631\u0627\u0641\u064A\n\u0645\u0639 \u062A\u0642\u0646\u064A\u0629 \u0627\u0644\u062A\u0639\u0631\u0641 \u0639\u0644\u0649 \u0627\u0644\u0648\u062C\u0648\u0647",
            "channel_name_menu": "\u0627\u0633\u0645 \u0627\u0644\u0642\u0646\u0627\u0629...", "channel_name_title": "\u0627\u0633\u0645 \u0627\u0644\u0642\u0646\u0627\u0629", "channel_name_desc": "\u064A\u0638\u0647\u0631 \u0647\u0630\u0627 \u0627\u0644\u0627\u0633\u0645 \u0639\u0644\u0649 \u062C\u0645\u064A\u0639 \u0627\u0644\u0639\u0631\u0648\u0636", "branding": "\u0647\u0648\u064A\u0629 \u0627\u0644\u0642\u0646\u0627\u0629...", "branding_title": "\u0647\u0648\u064A\u0629 \u0627\u0644\u0642\u0646\u0627\u0629",
            "channel_logo": "\u0634\u0639\u0627\u0631 \u0627\u0644\u0642\u0646\u0627\u0629", "browse": "\u062A\u0635\u0641\u062D...", "position": "\u0627\u0644\u0645\u0648\u0636\u0639", "size": "\u0627\u0644\u062D\u062C\u0645", "entry_anim": "\u062D\u0631\u0643\u0629 \u0627\u0644\u062F\u062E\u0648\u0644", "loop_anim": "\u062D\u0631\u0643\u0629 \u0645\u062A\u0643\u0631\u0631\u0629", "keep_logo_ads": "\u0625\u0628\u0642\u0627\u0621 \u0627\u0644\u0634\u0639\u0627\u0631 \u0623\u062B\u0646\u0627\u0621 \u0627\u0644\u0625\u0639\u0644\u0627\u0646\u0627\u062A",
            "channel_name_section": "\u0627\u0633\u0645 \u0627\u0644\u0642\u0646\u0627\u0629", "show_name_text": "\u0639\u0631\u0636 \u0627\u0644\u0627\u0633\u0645", "shape": "\u0627\u0644\u0634\u0643\u0644", "bg_color": "\u0627\u0644\u062E\u0644\u0641\u064A\u0629", "text_color": "\u0627\u0644\u0646\u0635", "border_color": "\u0627\u0644\u062D\u062F\u0648\u062F", "font_size": "\u062D\u062C\u0645 \u0627\u0644\u062E\u0637",
            "show_title_section": "\u0639\u0646\u0648\u0627\u0646 \u0627\u0644\u0628\u0631\u0646\u0627\u0645\u062C", "enable_title": "\u062A\u0641\u0639\u064A\u0644 \u0627\u0644\u0639\u0646\u0648\u0627\u0646", "title": "\u0627\u0644\u0639\u0646\u0648\u0627\u0646", "subtitle": "\u0627\u0644\u0639\u0646\u0648\u0627\u0646 \u0627\u0644\u0641\u0631\u0639\u064A",
            "show_title_shape": "\u0627\u0644\u0634\u0643\u0644", "show_title_bg": "\u0627\u0644\u062E\u0644\u0641\u064A\u0629", "show_title_text": "\u0627\u0644\u0646\u0635", "show_title_border": "\u0627\u0644\u062D\u062F\u0648\u062F",
            "cycle_talent_title": "\u062F\u0648\u0631\u0629 \u0627\u0644\u0645\u0648\u0647\u0628\u0629 / \u0627\u0644\u0639\u0646\u0648\u0627\u0646", "talent_display_dur": "\u0645\u062F\u0629 \u0627\u0644\u0639\u0631\u0636", "title_reappear": "\u062A\u0623\u062E\u064A\u0631 \u0627\u0644\u0639\u0648\u062F\u0629",
            "welcome": "\u0645\u0631\u062D\u0628\u0627", "enter_channel": "\u0623\u062F\u062E\u0644 \u0627\u0633\u0645 \u0642\u0646\u0627\u062A\u0643", "channel_overlay_desc": "\u0633\u064A\u0638\u0647\u0631 \u0647\u0630\u0627 \u0627\u0644\u0627\u0633\u0645 \u0639\u0644\u0649 \u0627\u0644\u0639\u0631\u0648\u0636", "start": "\u0628\u062F\u0621",
            "export_title": "\u062A\u0635\u062F\u064A\u0631 \u0627\u0644\u0645\u0644\u0641", "file_path": "\u0645\u0633\u0627\u0631 \u0627\u0644\u0645\u0644\u0641", "export": "\u062A\u0635\u062F\u064A\u0631", "import_title": "\u0627\u0633\u062A\u064A\u0631\u0627\u062F \u0627\u0644\u0645\u0644\u0641", "json_path": "\u0645\u0633\u0627\u0631 JSON", "import": "\u0627\u0633\u062A\u064A\u0631\u0627\u062F",
            "choose_logo": "\u0627\u062E\u062A\u0631 \u0634\u0639\u0627\u0631", "source_config": "\u0625\u0639\u062F\u0627\u062F \u0627\u0644\u0645\u0635\u062F\u0631...", "output_config": "\u0625\u0639\u062F\u0627\u062F \u0627\u0644\u0645\u062E\u0631\u062C\u0627\u062A...",
            "top_right": "\u0623\u0639\u0644\u0649 \u0627\u0644\u064A\u0645\u064A\u0646", "top_left": "\u0623\u0639\u0644\u0649 \u0627\u0644\u064A\u0633\u0627\u0631", "bottom_right": "\u0623\u0633\u0641\u0644 \u0627\u0644\u064A\u0645\u064A\u0646", "bottom_left": "\u0623\u0633\u0641\u0644 \u0627\u0644\u064A\u0633\u0627\u0631",
            "rec": "REC", "scoreboard": "\u0627\u0644\u0646\u062A\u064A\u062C\u0629", "weather": "\u0627\u0644\u0637\u0642\u0633", "subtitles": "\u062A\u0631\u062C\u0645\u0629", "social": "\u0627\u062C\u062A\u0645\u0627\u0639\u064A",
            "overlays_active": "OVERLAYS \u0646\u0634\u0637\u0629", "passthrough": "PASSTHROUGH", "programs": "\u0627\u0644\u0628\u0631\u0627\u0645\u062C", "current_program": "\u062D\u0627\u0644\u064A", "new_program": "\u0628\u0631\u0646\u0627\u0645\u062C \u062C\u062F\u064A\u062F", "configure": "\u062A\u0643\u0648\u064A\u0646...", "activate_overlays": "\u062A\u0641\u0639\u064A\u0644 \u0627\u0644\u0639\u0631\u0648\u0636", "deactivate_overlays": "\u062A\u0639\u0637\u064A\u0644 \u0627\u0644\u0639\u0631\u0648\u0636", "activate_overlays_desc": "\u0633\u064A\u062A\u0645 \u062A\u0641\u0639\u064A\u0644 \u0627\u0644\u0639\u0631\u0648\u0636.", "deactivate_overlays_desc": "\u0633\u064A\u062A\u0645 \u062A\u0639\u0637\u064A\u0644 \u0627\u0644\u0639\u0631\u0648\u0636.", "next_program": "\u0627\u0644\u0628\u0631\u0646\u0627\u0645\u062C \u0627\u0644\u062A\u0627\u0644\u064A"
        },
        "zh": { "file": "\u6587\u4EF6", "emission": "\u64AD\u51FA", "style": "\u98CE\u683C", "display": "\u663E\u793A", "tools": "\u5DE5\u5177", "help": "\u5E2E\u52A9", "language": "\u8BED\u8A00", "new_profile": "\u65B0\u914D\u7F6E...", "save_profile": "\u4FDD\u5B58\u914D\u7F6E", "export_profile": "\u5BFC\u51FA...", "import_profile": "\u5BFC\u5165...", "scan_hw": "\u626B\u63CF\u786C\u4EF6", "refresh_talents": "\u5237\u65B0\u4EBA\u7269", "quit": "\u9000\u51FA", "launch_stop": "\u5F00\u59CB / \u505C\u6B62", "fullscreen": "\u5168\u5C4F", "theme": "\u4EAE/\u6697\u4E3B\u9898", "setup_mode": "\u8BBE\u7F6E\u6A21\u5F0F", "live_mode": "\u76F4\u64AD\u6A21\u5F0F", "talent_mgmt": "\u4EBA\u7269\u7BA1\u7406...", "sdk_status": "SDK\u72B6\u6001...", "teleprompter": "\u63D0\u8BCD\u5668...", "macros": "\u5B8F\u547D\u4EE4...", "ticker": "\u6EDA\u52A8\u6761...", "stats": "\u7EDF\u8BA1...", "shortcuts": "\u5FEB\u6377\u952E", "about": "\u5173\u4E8E Prestige AI", "remote": "\u8FDC\u7A0B\u63A7\u5236...", "all_styles": "\u6240\u6709\u98CE\u683C...",
            "on_air": "\u76F4\u64AD", "bypass": "BYPASS", "overlays_on": "OVERLAYS ON", "overlays_off": "OVERLAYS OFF", "stop": "\u505C\u6B62", "stop_emission": "\u505C\u6B62\u64AD\u51FA", "launch_emission": "\u5F00\u59CB\u64AD\u51FA", "waiting": "\u7B49\u5F85\u68C0\u6D4B...", "detection": "\u68C0\u6D4B", "face": "\u4EBA\u8138", "faces": "\u4EBA\u8138", "output_live": "\u76F4\u64AD\u8F93\u51FA", "launch_vision": "\u542F\u52A8 Vision Engine \u67E5\u770B\u6D41",
            "next": "\u4E0B\u4E00\u6B65", "previous": "\u4E0A\u4E00\u6B65", "save": "\u4FDD\u5B58", "launch": "\u5F00\u59CB", "launch_emission_btn": "\u5F00\u59CB\u64AD\u51FA",
            "step_style": "\u98CE\u683C\u9009\u62E9", "step_talents": "\u4EBA\u7269\u7BA1\u7406", "step_source": "\u89C6\u9891\u6E90", "step_outputs": "\u8F93\u51FA", "step_test": "\u6D4B\u8BD5\u4E0E\u9A8C\u8BC1",
            "config_emission": "\u64AD\u51FA\u914D\u7F6E", "active_profile": "\u5F53\u524D\u914D\u7F6E", "modify": "\u4FEE\u6539...",
            "overlay_style": "\u8986\u76D6\u5C42\u98CE\u683C", "hover_preview": "\u60AC\u505C\u9884\u89C8", "preview": "\u9884\u89C8", "replay_anim": "\u91CD\u64AD\u52A8\u753B", "accent_color": "\u5F3A\u8C03\u8272", "opacity": "\u900F\u660E\u5EA6", "animation": "\u52A8\u753B", "multi_face": "\u591A\u4EBA\u8138", "delays": "\u5EF6\u8FDF", "enter": "\u8FDB\u5165", "exit": "\u9000\u51FA", "single_face": "1\u4EBA\u8138", "multi": "\u591A\u4EBA", "hide": "\u9690\u85CF",
            "emission_talents": "\u64AD\u51FA\u4EBA\u7269", "new_talent": "+ \u65B0\u4EBA\u7269", "refresh": "\u5237\u65B0", "ai_connected": "AI\u670D\u52A1\u5668\u5DF2\u8FDE\u63A5", "ai_disconnected": "AI\u670D\u52A1\u5668\u672A\u8FDE\u63A5", "talents_count": "\u4EBA\u7269", "no_embedding": "\u65E0embedding", "embedding_count": "\u4E2Aembedding", "photo": "\u7167\u7247", "delete": "\u5220\u9664", "no_talents": "\u65E0\u5DF2\u6CE8\u518C\u4EBA\u7269\n\u70B9\u51FB'+'\u6DFB\u52A0", "talent_register": "\u4EBA\u7269\u6CE8\u518C", "full_name": "\u5168\u540D", "role_function": "\u89D2\u8272/\u804C\u4F4D", "cancel": "\u53D6\u6D88", "register": "\u4FDD\u5B58", "create": "\u521B\u5EFA",
            "video_source": "\u89C6\u9891\u8F93\u5165\u6E90", "choose_source": "\u9009\u62E9AI\u5206\u6790\u7684\u89C6\u9891\u6E90\u3002", "ndi_sources": "\u53EF\u7528NDI\u6E90", "search": "\u641C\u7D22", "searching_ndi": "\u6B63\u5728\u641C\u7D22NDI\u6E90...", "connect": "\u8FDE\u63A5", "connected": "\u5DF2\u8FDE\u63A5", "no_ndi": "\u672A\u68C0\u6D4B\u5230NDI\u6E90", "resolution": "\u5206\u8FA8\u7387",
            "broadcast_outputs": "\u64AD\u51FA\u8F93\u51FA (TV)", "broadcast_desc": "\u4E13\u4E1A\u7535\u89C6\u64AD\u51FA\u8F93\u51FA\u3002", "social_outputs": "\u793E\u4EA4\u5A92\u4F53\uFF08\u540C\u6B65\u76F4\u64AD\uFF09", "social_desc": "\u540C\u65F6\u5728\u591A\u4E2A\u5E73\u53F0\u76F4\u64AD\u3002", "stream_key": "\u76F4\u64AD\u5BC6\u94A5", "key_configured": "\u5BC6\u94A5\u5DF2\u914D\u7F6E", "not_configured": "\u672A\u914D\u7F6E", "output_summary": "\u6D3B\u52A8\u8F93\u51FA\u6458\u8981", "no_output": "\u672A\u914D\u7F6E\u8F93\u51FA", "social_networks": "\u4E2A\u793E\u4EA4\u7F51\u7EDC",
            "live_test": "\u5B9E\u65F6\u6D4B\u8BD5", "click_test": "\u70B9\u51FB\u5F00\u59CB\u6D4B\u8BD5", "launch_test": "\u5F00\u59CB\u6D4B\u8BD5\uFF08\u6444\u50CF\u5934\uFF09", "pre_checklist": "\u64AD\u51FA\u524D\u68C0\u67E5", "style_chosen": "\u5DF2\u9009\u62E9\u8986\u76D6\u5C42\u98CE\u683C", "talent_added": "\u81F3\u5C111\u4E2A\u4EBA\u7269", "source_configured": "\u89C6\u9891\u6E90\u5DF2\u914D\u7F6E", "output_active": "\u81F3\u5C111\u4E2A\u8F93\u51FA", "ai_server": "AI\u670D\u52A1\u5668\u5DF2\u8FDE\u63A5", "ready": "\u5DF2\u51C6\u5907\u5C31\u7EEA", "not_ready": "\u914D\u7F6E\u4E0D\u5B8C\u6574", "can_launch": "\u53EF\u4EE5\u5F00\u59CB\u64AD\u51FA", "complete_steps": "\u5B8C\u6210\u4EE5\u4E0A\u6B65\u9AA4",
            "launch_question": "\u5F00\u59CB\u64AD\u51FA\uFF1F", "live_switch": "\u754C\u9762\u5C06\u5207\u6362\u5230\u76F4\u64AD\u6A21\u5F0F\u3002", "stop_question": "\u505C\u6B62\u64AD\u51FA\uFF1F", "stop_desc": "\u6240\u6709\u8986\u76D6\u5C42\u548C\u8F93\u51FA\u5C06\u88AB\u7981\u7528\u3002", "continue": "\u7EE7\u7EED",
            "new_profile_title": "\u65B0\u914D\u7F6E", "emission_name": "\u64AD\u51FA\u540D\u79F0", "profile_saved": "\u914D\u7F6E\u5DF2\u4FDD\u5B58", "hw_scan_done": "\u786C\u4EF6\u626B\u63CF\u5B8C\u6210",
            "sdk_broadcast_status": "\u64AD\u51FASDK\u72B6\u6001", "installed": "\u5DF2\u5B89\u88C5", "close": "\u5173\u95ED",
            "shortcuts_title": "\u5FEB\u6377\u952E", "about_desc": "\u4E13\u4E1A\u64AD\u51FA\u8F6F\u4EF6\nAI\u4EBA\u8138\u8BC6\u522B",
            "channel_name_menu": "\u9891\u9053\u540D\u79F0...", "channel_name_title": "\u9891\u9053\u540D\u79F0", "channel_name_desc": "\u6B64\u540D\u79F0\u663E\u793A\u5728\u6240\u6709\u76F4\u64AD\u8986\u76D6\u5C42\u4E0A", "branding": "\u9891\u9053\u5305\u88C5...", "branding_title": "\u9891\u9053\u5305\u88C5",
            "channel_logo": "\u9891\u9053\u6807\u5FD7", "browse": "\u6D4F\u89C8...", "position": "\u4F4D\u7F6E", "size": "\u5927\u5C0F", "entry_anim": "\u8FDB\u5165\u52A8\u753B", "loop_anim": "\u5FAA\u73AF\u52A8\u753B", "keep_logo_ads": "\u5E7F\u544A\u65F6\u4FDD\u7559\u6807\u5FD7",
            "channel_name_section": "\u9891\u9053\u540D\u79F0", "show_name_text": "\u663E\u793A\u6587\u5B57\u540D\u79F0", "shape": "\u5F62\u72B6", "bg_color": "\u80CC\u666F", "text_color": "\u6587\u5B57", "border_color": "\u8FB9\u6846", "font_size": "\u5B57\u4F53\u5927\u5C0F",
            "show_title_section": "\u8282\u76EE\u6807\u9898", "enable_title": "\u542F\u7528\u6C38\u4E45\u6807\u9898", "title": "\u6807\u9898", "subtitle": "\u526F\u6807\u9898",
            "show_title_shape": "\u5F62\u72B6", "show_title_bg": "\u80CC\u666F", "show_title_text": "\u6587\u5B57", "show_title_border": "\u8FB9\u6846",
            "cycle_talent_title": "\u4EBA\u7269/\u6807\u9898\u5FAA\u73AF", "talent_display_dur": "\u4EBA\u7269\u663E\u793A\u65F6\u957F", "title_reappear": "\u6807\u9898\u91CD\u73B0\u5EF6\u8FDF",
            "welcome": "\u6B22\u8FCE", "enter_channel": "\u8F93\u5165\u60A8\u7684\u9891\u9053\u540D\u79F0", "channel_overlay_desc": "\u6B64\u540D\u79F0\u5C06\u663E\u793A\u5728\u6240\u6709\u76F4\u64AD\u8986\u76D6\u5C42\u4E0A", "start": "\u5F00\u59CB",
            "export_title": "\u5BFC\u51FA\u914D\u7F6E", "file_path": "\u6587\u4EF6\u8DEF\u5F84", "export": "\u5BFC\u51FA", "import_title": "\u5BFC\u5165\u914D\u7F6E", "json_path": "JSON\u6587\u4EF6\u8DEF\u5F84", "import": "\u5BFC\u5165",
            "choose_logo": "\u9009\u62E9\u6807\u5FD7", "source_config": "\u6E90\u914D\u7F6E...", "output_config": "\u8F93\u51FA\u914D\u7F6E...",
            "top_right": "\u53F3\u4E0A", "top_left": "\u5DE6\u4E0A", "bottom_right": "\u53F3\u4E0B", "bottom_left": "\u5DE6\u4E0B",
            "rec": "REC", "scoreboard": "\u6BD4\u5206", "weather": "\u5929\u6C14", "subtitles": "\u5B57\u5E55", "social": "\u793E\u4EA4",
            "overlays_active": "\u8986\u76D6\u5C42\u5DF2\u6FC0\u6D3B", "passthrough": "PASSTHROUGH", "programs": "\u8282\u76EE", "current_program": "\u5F53\u524D", "new_program": "\u65B0\u8282\u76EE", "configure": "\u914D\u7F6E...", "activate_overlays": "\u6FC0\u6D3B\u8986\u76D6\u5C42", "deactivate_overlays": "\u505C\u7528\u8986\u76D6\u5C42", "activate_overlays_desc": "\u8986\u76D6\u5C42\u5C06\u88AB\u6FC0\u6D3B\u3002\n\u4FE1\u53F7\u59CB\u7EC8\u6D41\u901A\u3002", "deactivate_overlays_desc": "\u8986\u76D6\u5C42\u5C06\u88AB\u505C\u7528\u3002\n\u4FE1\u53F7\u7EE7\u7EED\u76F4\u901A\u3002", "next_program": "\u4E0B\u4E00\u4E2A\u8282\u76EE"
        }
    })

    function t(key) {
        var langMap = translations[lang];
        if (langMap && langMap[key]) return langMap[key];
        // Fallback to French
        var frMap = translations["fr"];
        if (frMap && frMap[key]) return frMap[key];
        return key;
    }

    // ── Theme toggle (Feature 11) ─────────────────────────
    property bool darkMode: true

    // ── Global font ────────────────────────────────────────
    font.family: "Helvetica Neue"

    // ── Menu Bar ─────────────────────────────────────────
    menuBar: MenuBar {
        id: appMenuBar

        background: Rectangle { color: window.darkMode ? "#0D0D10" : "#E0E0E6"; Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.08) } }

        delegate: MenuBarItem {
            id: menuBarDelegate
            contentItem: Label {
                text: menuBarDelegate.text
                font.pixelSize: 13
                color: menuBarDelegate.highlighted ? "white" : "#999"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: menuBarDelegate.highlighted ? Qt.rgba(91/255,79/255,219/255,0.2) : "transparent"
                Behavior on color { ColorAnimation { duration: 100 } }
            }
        }

        // ── Fichier ────────────────────────────────────────
        Menu {
            title: window.t("file")
            background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; border.color: window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.12); radius: 6 }

            MenuItem {
                text: window.t("new_profile")
                onTriggered: newProfileDialog.open()
            }
            MenuItem {
                text: window.t("save_profile")
                onTriggered: { setupController.saveProfile(); savedNotif.show() }
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: window.t("export_profile")
                onTriggered: exportProfileDialog.open()
            }
            MenuItem {
                text: window.t("import_profile")
                onTriggered: importProfileDialog.open()
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: window.t("channel_name_menu")
                onTriggered: channelNameDialog.open()
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: window.t("quit")
                onTriggered: Qt.quit()
            }
        }

        // ── Style ─────────────────────────────────────────
        Menu {
            title: window.t("style")
            background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; border.color: window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.12); radius: 6 }

            MenuItem { text: "BFM TV"; checkable: true; checked: setupController.selectedStyle === "bfm"; onTriggered: setupController.selectedStyle = "bfm" }
            MenuItem { text: "LCI"; checkable: true; checked: setupController.selectedStyle === "lci"; onTriggered: setupController.selectedStyle = "lci" }
            MenuItem { text: "France 2"; checkable: true; checked: setupController.selectedStyle === "france2"; onTriggered: setupController.selectedStyle = "france2" }
            MenuItem { text: "CNN"; checkable: true; checked: setupController.selectedStyle === "cnn"; onTriggered: setupController.selectedStyle = "cnn" }
            MenuItem { text: "BBC News"; checkable: true; checked: setupController.selectedStyle === "bbc"; onTriggered: setupController.selectedStyle = "bbc" }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem { text: "Cinema / Festival"; checkable: true; checked: setupController.selectedStyle === "cinema"; onTriggered: setupController.selectedStyle = "cinema" }
            MenuItem { text: "Luxury / Prestige"; checkable: true; checked: setupController.selectedStyle === "luxury"; onTriggered: setupController.selectedStyle = "luxury" }
            MenuItem { text: "Tech / Innovation"; checkable: true; checked: setupController.selectedStyle === "tech"; onTriggered: setupController.selectedStyle = "tech" }
            MenuItem { text: "Minimaliste"; checkable: true; checked: setupController.selectedStyle === "minimal"; onTriggered: setupController.selectedStyle = "minimal" }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem { text: "Breaking News"; checkable: true; checked: setupController.selectedStyle === "breaking"; onTriggered: setupController.selectedStyle = "breaking" }
            MenuItem {
                text: window.t("all_styles")
                onTriggered: { navPanelDrawer.activePanel = 3; navPanelDrawer.open() }
            }
        }

        // ── Affichage ─────────────────────────────────────
        Menu {
            title: window.t("display")
            background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; border.color: window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.12); radius: 6 }

            MenuItem {
                text: window.t("fullscreen")
                checkable: true
                checked: window.visibility === Window.FullScreen
                onTriggered: {
                    if (window.visibility === Window.FullScreen) window.showNormal()
                    else window.showFullScreen()
                }
            }
            MenuItem {
                text: window.t("theme")
                checkable: true
                checked: !window.darkMode
                onTriggered: {
                    window.darkMode = !window.darkMode
                }
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            Menu {
                title: window.t("language")
                MenuItem { text: "Fran\u00E7ais"; checkable: true; checked: window.lang === "fr"; onTriggered: mainWindow.language = "fr" }
                MenuItem { text: "English"; checkable: true; checked: window.lang === "en"; onTriggered: mainWindow.language = "en" }
                MenuItem { text: "Espa\u00F1ol"; checkable: true; checked: window.lang === "es"; onTriggered: mainWindow.language = "es" }
                MenuItem { text: "\u0627\u0644\u0639\u0631\u0628\u064A\u0629"; checkable: true; checked: window.lang === "ar"; onTriggered: mainWindow.language = "ar" }
                MenuItem { text: "\u4E2D\u6587"; checkable: true; checked: window.lang === "zh"; onTriggered: mainWindow.language = "zh" }
            }
        }

        // ── Outils ────────────────────────────────────────
        Menu {
            title: window.t("tools")
            background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; border.color: window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.12); radius: 6 }

            MenuItem {
                text: window.t("source_config")
                onTriggered: { navPanelDrawer.activePanel = 0; navPanelDrawer.open() }
            }
            MenuItem {
                text: window.t("output_config")
                onTriggered: { navPanelDrawer.activePanel = 1; navPanelDrawer.open() }
            }
            MenuItem {
                text: window.t("talent_mgmt")
                onTriggered: { navPanelDrawer.activePanel = 2; navPanelDrawer.open() }
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: window.t("branding")
                onTriggered: { navPanelDrawer.activePanel = 3; navPanelDrawer.open() }
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: window.t("scan_hw")
                onTriggered: { hardwareScanner.scan(); hardwareScanNotif.show() }
            }
            MenuItem {
                text: window.t("sdk_status")
                onTriggered: sdkStatusDialog.open()
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: window.t("remote") + (webRemote.running ? " (:" + webRemote.port + ")" : "")
                onTriggered: {
                    if (webRemote.running) webRemote.stop()
                    else webRemote.start()
                }
            }
        }

        // ── Aide ──────────────────────────────────────────
        Menu {
            title: window.t("help")
            background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; border.color: window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.12); radius: 6 }

            MenuItem {
                text: window.t("shortcuts")
                onTriggered: shortcutsDialog.open()
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: window.t("about")
                onTriggered: aboutDialog.open()
            }
            MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Qt.rgba(1,1,1,0.06) } }
            MenuItem {
                text: "Licence..."
                onTriggered: licenseInfoDialog.open()
            }
        }
    }

    // ── Keyboard shortcuts ──────────────────────────────────
    Shortcut { sequence: "Ctrl+N"; onActivated: newProfileDialog.open() }
    Shortcut { sequence: "Ctrl+S"; onActivated: { setupController.saveProfile(); savedNotif.show() } }
    Shortcut { sequence: "Ctrl+Q"; onActivated: Qt.quit() }
    Shortcut { sequence: "F5"; onActivated: { if (mainWindow.overlaysActive) stopConfirmDialog.open(); else confirmLaunchDialog.open() } }
    Shortcut { sequence: "Ctrl+Right"; onActivated: { var next = mainWindow.activeProgram + 1; if (next >= mainWindow.programList.length) next = 0; mainWindow.switchProgram(next) } }
    Shortcut { sequence: "Ctrl+0"; onActivated: mainWindow.switchProgram(mainWindow.programList.length - 1) }
    Shortcut { sequence: "F11"; onActivated: { if (window.visibility === Window.FullScreen) window.showNormal(); else window.showFullScreen() } }
    Shortcut { sequence: "Ctrl+1"; onActivated: { navPanelDrawer.activePanel = 0; navPanelDrawer.open() } }
    Shortcut { sequence: "Ctrl+2"; onActivated: mainWindow.toggleOverlays() }
    Shortcut { sequence: "Ctrl+T"; onActivated: { navPanelDrawer.activePanel = 2; navPanelDrawer.open() } }

    // ── MAIN LAYOUT: Nav bar (left) + Content (right) ─────
    RowLayout {
        anchors.fill: parent; spacing: 0

        // ── LEFT NAVIGATION BAR (60px, always visible) ──
        Rectangle {
            Layout.preferredWidth: 60; Layout.fillHeight: true
            color: window.darkMode ? "#08080C" : "#E0E0E6"

            ColumnLayout {
                anchors.fill: parent; anchors.topMargin: 8; anchors.bottomMargin: 8; spacing: 4

                // Nav buttons - each opens a drawer
                Repeater {
                    model: ListModel {
                        ListElement { icon: "\uD83D\uDCFA"; tooltip: "Entrees"; panel: 0 }
                        ListElement { icon: "\uD83D\uDCE1"; tooltip: "Sorties"; panel: 1 }
                        ListElement { icon: "\uD83D\uDC64"; tooltip: "Talents"; panel: 2 }
                        ListElement { icon: "\uD83C\uDFA8"; tooltip: "Style"; panel: 3 }
                        ListElement { icon: "\uD83D\uDD27"; tooltip: "Outils"; panel: 4 }
                        ListElement { icon: "\u2699"; tooltip: "Parametres"; panel: 5 }
                    }

                    Rectangle {
                        Layout.preferredWidth: 48; Layout.preferredHeight: 48; Layout.alignment: Qt.AlignHCenter
                        radius: 10
                        color: navPanelDrawer.visible && navPanelDrawer.activePanel === model.panel
                            ? (window.darkMode ? Qt.rgba(91/255,79/255,219/255,0.2) : Qt.rgba(91/255,79/255,219/255,0.15))
                            : (navBtnMa.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : "transparent")
                        border.color: navPanelDrawer.visible && navPanelDrawer.activePanel === model.panel ? "#5B4FDB" : "transparent"

                        Label {
                            anchors.centerIn: parent
                            text: model.icon
                            font.pixelSize: 20
                        }

                        ToolTip {
                            visible: navBtnMa.containsMouse && !navPanelDrawer.visible
                            text: model.tooltip
                            delay: 500
                        }

                        MouseArea {
                            id: navBtnMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (navPanelDrawer.visible && navPanelDrawer.activePanel === model.panel) {
                                    navPanelDrawer.close()
                                } else {
                                    navPanelDrawer.activePanel = model.panel
                                    navPanelDrawer.open()
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }  // Spacer

                // Channel logo/initial at bottom of nav
                Rectangle {
                    Layout.preferredWidth: 40; Layout.preferredHeight: 40; Layout.alignment: Qt.AlignHCenter
                    radius: 8
                    color: "#5B4FDB"
                    Label {
                        anchors.centerIn: parent
                        text: (configManager.channelName || "P").charAt(0).toUpperCase()
                        font.pixelSize: 18; font.weight: Font.Bold; color: "white"
                    }
                }
            }
        }

        // Nav bar separator
        Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08) }

        // ── MAIN CONTENT (program bar + preview) ──
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0

            // ── Slim program bar (top, 42px) ──────────────────
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 42
                color: window.darkMode ? "#0D0D10" : "#E0E0E6"

                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 12

                    // Channel name
                    Label {
                        text: configManager.channelName || "PRESTIGE AI"
                        font.pixelSize: 13; font.weight: Font.Bold
                        color: window.darkMode ? "white" : "#1A1A1A"
                    }

                    Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 20; color: window.darkMode ? "#333" : "#CCC" }

                    // Overlay status indicator
                    Rectangle {
                        Layout.preferredWidth: statusRow.implicitWidth + 16; Layout.preferredHeight: 26; radius: 13
                        color: mainWindow.overlaysActive ? Qt.rgba(29/255,185/255,84/255,0.15) : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04))
                        border.color: mainWindow.overlaysActive ? Qt.rgba(29/255,185/255,84/255,0.3) : "transparent"
                        RowLayout {
                            id: statusRow; anchors.centerIn: parent; spacing: 6
                            Rectangle { Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: mainWindow.overlaysActive ? "#1DB954" : (window.darkMode ? "#555" : "#999") }
                            Label { text: mainWindow.overlaysActive ? window.t("overlays_active") : window.t("passthrough"); font.pixelSize: 10; font.weight: Font.DemiBold; color: mainWindow.overlaysActive ? "#1DB954" : (window.darkMode ? "#888" : "#666") }
                        }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: mainWindow.toggleOverlays() }
                    }

                    Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 20; color: window.darkMode ? "#333" : "#CCC" }

                    // Program selector (compact ComboBox-like)
                    Label { text: window.t("programs") + ":"; font.pixelSize: 10; color: window.darkMode ? "#888" : "#666" }

                    // Program buttons (horizontal, compact)
                    Flickable {
                        Layout.fillWidth: true; Layout.preferredHeight: 28
                        contentWidth: progRow.implicitWidth; clip: true
                        flickableDirection: Flickable.HorizontalFlick

                        Row {
                            id: progRow; spacing: 4
                            Repeater {
                                model: mainWindow.programList
                                Rectangle {
                                    width: progLabel.implicitWidth + 16; height: 26; radius: 13
                                    color: mainWindow.activeProgram === index ? "#5B4FDB" : (progMa.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : (window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)))
                                    Label { id: progLabel; anchors.centerIn: parent; text: modelData; font.pixelSize: 10; color: mainWindow.activeProgram === index ? "white" : (window.darkMode ? "#CCC" : "#333") }
                                    MouseArea { id: progMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: mainWindow.switchProgram(index) }
                                }
                            }
                        }
                    }
                }

                // Bottom border
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08) }
            }

            // ── Main content: LiveView always visible ─────────
            LiveView { id: liveView; Layout.fillWidth: true; Layout.fillHeight: true }
        }
    }

    // ── Navigation Panel Drawer ──────────────────────────
    Drawer {
        id: navPanelDrawer
        width: 420; height: window.height
        edge: Qt.LeftEdge
        modal: false

        property int activePanel: 0

        background: Rectangle {
            color: window.darkMode ? "#0D0D12" : "#F5F5FA"
            border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08)
        }

        ColumnLayout {
            anchors.fill: parent; spacing: 0

            // Header with title + close
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 50
                color: window.darkMode ? "#0A0A0F" : "#EAEAEF"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 12
                    Label {
                        text: ["Entrees", "Sorties", "Talents", "Style & Habillage", "Outils", "Parametres"][navPanelDrawer.activePanel]
                        font.pixelSize: 15; font.weight: Font.Bold
                        color: window.darkMode ? "white" : "#1A1A1A"
                        Layout.fillWidth: true
                    }
                    Rectangle {
                        Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 14
                        color: closePanelMa.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)) : "transparent"
                        Label { anchors.centerIn: parent; text: "\u2715"; font.pixelSize: 14; color: window.darkMode ? "#888" : "#666" }
                        MouseArea { id: closePanelMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: navPanelDrawer.close() }
                    }
                }
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08) }
            }

            // Panel content (switches based on activePanel)
            StackLayout {
                Layout.fillWidth: true; Layout.fillHeight: true
                currentIndex: navPanelDrawer.activePanel

                // Panel 0: Entrees
                SourceStep { }

                // Panel 1: Sorties
                OutputStep { }

                // Panel 2: Talents
                TalentStep { }

                // Panel 3: Style & Habillage
                Item {
                    Flickable {
                        anchors.fill: parent; contentHeight: styleHabCol.implicitHeight; clip: true
                        flickableDirection: Flickable.VerticalFlick
                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                        ColumnLayout {
                            id: styleHabCol; width: parent.width; spacing: 8

                            // Embed StyleStep content
                            StyleStep { Layout.fillWidth: true; Layout.preferredHeight: 500 }

                            // Branding controls (from old brandingDrawer)
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#DDD"; Layout.leftMargin: 8; Layout.rightMargin: 8 }

                            // ── LOGO SECTION ──────────────────────────────
                            Label { text: window.t("channel_logo"); font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 8 }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8; Layout.rightMargin: 8
                                TextField { id: logoPathField; Layout.fillWidth: true; text: setupController.channelLogoPath; onTextChanged: setupController.channelLogoPath = text; placeholderText: "logo.png ou logo.gif"; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" } }
                                Rectangle {
                                    Layout.preferredWidth: 90; Layout.preferredHeight: 30; radius: 4; color: "#5B4FDB"
                                    Label { anchors.centerIn: parent; text: window.t("browse"); color: "white"; font.pixelSize: 11 }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: logoFileDialogLoader.active = true }
                                }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Position:"; color: window.darkMode ? "#999" : "#666" }
                                ComboBox {
                                    id: logoPosCb
                                    model: ["Haut droite", "Haut gauche", "Bas droite", "Bas gauche"]
                                    onCurrentIndexChanged: { var pos = ["top_right","top_left","bottom_right","bottom_left"]; setupController.channelLogoPosition = pos[currentIndex] }
                                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                    contentItem: Label { text: logoPosCb.currentText; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                                }
                            }
                            RowLayout { spacing: 4; Layout.leftMargin: 8
                                Label { text: "Decalage X:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                                Slider { from: -200; to: 200; stepSize: 1; value: setupController.channelLogoOffsetX; Layout.fillWidth: true; onMoved: setupController.channelLogoOffsetX = value }
                                Label { text: setupController.channelLogoOffsetX + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 40 }
                            }
                            RowLayout { spacing: 4; Layout.leftMargin: 8
                                Label { text: "Decalage Y:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                                Slider { from: -200; to: 200; stepSize: 1; value: setupController.channelLogoOffsetY; Layout.fillWidth: true; onMoved: setupController.channelLogoOffsetY = value }
                                Label { text: setupController.channelLogoOffsetY + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 40 }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Taille:"; color: window.darkMode ? "#999" : "#666" }
                                Slider { id: logoSizeSlider; from: 30; to: 120; value: setupController.channelLogoSize; onMoved: setupController.channelLogoSize = value }
                                Label { text: Math.round(logoSizeSlider.value) + "px"; color: window.darkMode ? "#999" : "#666" }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Animation entree:"; color: window.darkMode ? "#999" : "#666" }
                                ComboBox {
                                    id: logoEntryCb
                                    model: ["Aucune", "Glisser gauche", "Glisser droite", "Glisser haut", "Glisser bas", "Fondu", "Zoom", "Balayage"]
                                    currentIndex: { var m = ["none","slide_left","slide_right","slide_up","slide_down","fade","scale","wipe"]; return Math.max(0, m.indexOf(setupController.logoEntryAnim)); }
                                    onCurrentIndexChanged: { var m = ["none","slide_left","slide_right","slide_up","slide_down","fade","scale","wipe"]; setupController.logoEntryAnim = m[currentIndex] }
                                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                    contentItem: Label { text: logoEntryCb.currentText; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                                }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Animation boucle:"; color: window.darkMode ? "#999" : "#666" }
                                ComboBox {
                                    id: logoLoopCb
                                    model: ["Aucune", "Pulsation", "Brillance", "Scintillement", "Rebond", "Rotation"]
                                    currentIndex: { var m = ["none","pulse","glow","shimmer","bounce","rotate"]; return Math.max(0, m.indexOf(setupController.logoLoopAnim)); }
                                    onCurrentIndexChanged: { var m = ["none","pulse","glow","shimmer","bounce","rotate"]; setupController.logoLoopAnim = m[currentIndex] }
                                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                    contentItem: Label { text: logoLoopCb.currentText; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                                }
                            }
                            Switch { text: window.t("keep_logo_ads"); checked: setupController.keepLogoDuringAds; onToggled: setupController.keepLogoDuringAds = checked; leftPadding: 8 }

                            // ── CHANNEL NAME SECTION ──────────────────────
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#DDD"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
                            Label { text: window.t("channel_name_section"); font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 8 }
                            Switch { text: window.t("show_name_text"); checked: setupController.showChannelNameText; onToggled: setupController.showChannelNameText = checked; leftPadding: 8 }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Forme:"; color: window.darkMode ? "#999" : "#666" }
                                ComboBox {
                                    id: nameShapeCb
                                    model: ["Rectangle", "Carre", "Pilule", "Oblique", "Sans cadre"]
                                    currentIndex: { var m = ["rectangle","square","pill","angled","frameless"]; return Math.max(0, m.indexOf(setupController.channelNameShape)); }
                                    onCurrentIndexChanged: { var m = ["rectangle","square","pill","angled","frameless"]; setupController.channelNameShape = m[currentIndex] }
                                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                    contentItem: Label { text: nameShapeCb.currentText; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                                }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Fond:"; color: window.darkMode ? "#999" : "#666" }
                                Repeater {
                                    model: ["#CC0000", "#0055AA", "#222222", "#006633", "#8B00FF", "#FF6600", "#333333"]
                                    Rectangle {
                                        width: 24; height: 24; radius: 12; color: modelData; border.color: setupController.channelNameBgColor === modelData ? "white" : "transparent"; border.width: 2
                                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.channelNameBgColor = modelData }
                                    }
                                }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Texte:"; color: window.darkMode ? "#999" : "#666" }
                                Repeater {
                                    model: ["#FFFFFF", "#000000", "#FFDD00", "#00FFCC", "#FF4444"]
                                    Rectangle {
                                        width: 24; height: 24; radius: 12; color: modelData; border.color: setupController.channelNameTextColor === modelData ? "#5B4FDB" : (window.darkMode ? "#444" : "#BBB"); border.width: 2
                                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.channelNameTextColor = modelData }
                                    }
                                }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Bordure:"; color: window.darkMode ? "#999" : "#666" }
                                Repeater {
                                    model: ["#FFFFFF", "#000000", "#CC0000", "#FFDD00", "#0055AA", "transparent"]
                                    Rectangle {
                                        width: 24; height: 24; radius: 12; color: modelData === "transparent" ? "gray" : modelData; border.color: setupController.channelNameBorderColor === modelData ? "#5B4FDB" : (window.darkMode ? "#444" : "#BBB"); border.width: 2
                                        Label { anchors.centerIn: parent; text: modelData === "transparent" ? "X" : ""; font.pixelSize: 10; color: "white" }
                                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.channelNameBorderColor = modelData }
                                    }
                                }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Taille police:"; color: window.darkMode ? "#999" : "#666" }
                                Slider { id: nameFontSlider; from: 8; to: 36; stepSize: 1; value: setupController.channelNameFontSize; onMoved: setupController.channelNameFontSize = value }
                                Label { text: Math.round(nameFontSlider.value) + "pt"; color: window.darkMode ? "#999" : "#666" }
                            }
                            RowLayout { spacing: 4; Layout.leftMargin: 8
                                Label { text: "Decalage X:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                                Slider { from: -200; to: 200; stepSize: 1; value: setupController.channelNameOffsetX; Layout.fillWidth: true; onMoved: setupController.channelNameOffsetX = value }
                                Label { text: setupController.channelNameOffsetX + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 40 }
                            }
                            RowLayout { spacing: 4; Layout.leftMargin: 8
                                Label { text: "Decalage Y:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                                Slider { from: -200; to: 200; stepSize: 1; value: setupController.channelNameOffsetY; Layout.fillWidth: true; onMoved: setupController.channelNameOffsetY = value }
                                Label { text: setupController.channelNameOffsetY + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 40 }
                            }

                            // ── SHOW TITLE SECTION ────────────────────────
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#DDD"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
                            Label { text: window.t("show_title_section"); font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 8 }
                            Switch { text: window.t("enable_title"); checked: setupController.showTitleEnabled; onToggled: setupController.showTitleEnabled = checked; leftPadding: 8 }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8; Layout.rightMargin: 8
                                Label { text: "Titre:"; color: window.darkMode ? "#999" : "#666" }
                                TextField { Layout.fillWidth: true; text: setupController.showTitle; onTextChanged: setupController.showTitle = text; placeholderText: "JOURNAL DE 20H"; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" } }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8; Layout.rightMargin: 8
                                Label { text: "Sous-titre:"; color: window.darkMode ? "#999" : "#666" }
                                TextField { Layout.fillWidth: true; text: setupController.showSubtitle; onTextChanged: setupController.showSubtitle = text; placeholderText: "Edition speciale"; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" } }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Position:"; color: window.darkMode ? "#999" : "#666" }
                                ComboBox {
                                    id: titlePosCb
                                    model: ["Bas gauche", "Bas droite", "Haut gauche", "Haut droite"]
                                    onCurrentIndexChanged: { var pos = ["bottom_left","bottom_right","top_left","top_right"]; setupController.showTitlePosition = pos[currentIndex] }
                                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                    contentItem: Label { text: titlePosCb.currentText; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                                }
                            }
                            RowLayout { spacing: 4; Layout.leftMargin: 8
                                Label { text: "Decalage X:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                                Slider { from: -200; to: 200; stepSize: 1; value: setupController.showTitleOffsetX; Layout.fillWidth: true; onMoved: setupController.showTitleOffsetX = value }
                                Label { text: setupController.showTitleOffsetX + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 40 }
                            }
                            RowLayout { spacing: 4; Layout.leftMargin: 8
                                Label { text: "Decalage Y:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                                Slider { from: -200; to: 200; stepSize: 1; value: setupController.showTitleOffsetY; Layout.fillWidth: true; onMoved: setupController.showTitleOffsetY = value }
                                Label { text: setupController.showTitleOffsetY + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 40 }
                            }

                            // ── TIMING SECTION ────────────────────────────
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#DDD"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
                            Label { text: window.t("cycle_talent_title"); font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 8 }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Duree affichage talent:"; color: window.darkMode ? "#999" : "#666" }
                                Slider { id: talentDurSlider; from: 3; to: 30; stepSize: 1; value: setupController.talentDisplayDurationSec; onMoved: setupController.talentDisplayDurationSec = value }
                                Label { text: Math.round(talentDurSlider.value) + "s"; color: window.darkMode ? "#999" : "#666" }
                            }
                            RowLayout {
                                spacing: 8; Layout.leftMargin: 8
                                Label { text: "Delai retour titre:"; color: window.darkMode ? "#999" : "#666" }
                                Slider { id: titleDelaySlider; from: 1; to: 10; stepSize: 1; value: setupController.titleReappearDelaySec; onMoved: setupController.titleReappearDelaySec = value }
                                Label { text: Math.round(titleDelaySlider.value) + "s"; color: window.darkMode ? "#999" : "#666" }
                            }

                            Item { Layout.preferredHeight: 16 }
                        }
                    }
                }

                // Panel 4: Outils
                Item {
                    ColumnLayout {
                        anchors.fill: parent; spacing: 0

                        // Tool tabs
                        RowLayout {
                            Layout.fillWidth: true; Layout.preferredHeight: 36; spacing: 0
                            Repeater {
                                model: ["\u26BD Score", "\u2601 Meteo", "\uD83D\uDCAC Ticker", "\uD83C\uDF99 ST", "\uD83D\uDCF1 Chat", "\uD83D\uDCDD Prompt"]
                                Rectangle {
                                    Layout.fillWidth: true; Layout.preferredHeight: 36
                                    color: toolsStack.currentIndex === index ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : "transparent"
                                    Label { anchors.centerIn: parent; text: modelData; font.pixelSize: 9; color: toolsStack.currentIndex === index ? (window.darkMode ? "white" : "#1A1A1A") : (window.darkMode ? "#666" : "#999") }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: toolsStack.currentIndex = index }
                                }
                            }
                        }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

                        StackLayout {
                            id: toolsStack; Layout.fillWidth: true; Layout.fillHeight: true
                            ScoreboardPanel {}
                            WeatherPanel {}
                            TickerPanel {}
                            SubtitlePanel {}
                            SocialChatPanel {}
                            TeleprompterPanel {}
                        }
                    }
                }

                // Panel 5: Parametres
                Item {
                    Flickable {
                        anchors.fill: parent; contentHeight: settingsCol.implicitHeight; clip: true
                        flickableDirection: Flickable.VerticalFlick
                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                        ColumnLayout {
                            id: settingsCol; width: parent.width; spacing: 10
                            Item { implicitHeight: 8 }

                            // Licence
                            Label { text: "Licence"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            Rectangle {
                                width: parent.width - 24; height: 50; x: 12; radius: 8
                                color: window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)
                                RowLayout {
                                    anchors.fill: parent; anchors.margins: 12; spacing: 8
                                    Rectangle { Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: licenseManager.isActivated ? "#1DB954" : "#CC3333" }
                                    Label { text: licenseManager.licenseType + " -- " + licenseManager.expirationDate; font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"; Layout.fillWidth: true }
                                }
                            }

                            // Langue
                            Label { text: "Langue"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            RowLayout { spacing: 4; Layout.leftMargin: 12
                                Repeater {
                                    model: ["FR", "EN", "ES", "AR", "ZH"]
                                    Rectangle {
                                        Layout.preferredWidth: 40; Layout.preferredHeight: 28; radius: 6
                                        color: window.lang === modelData.toLowerCase() ? "#5B4FDB" : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04))
                                        Label { anchors.centerIn: parent; text: modelData; font.pixelSize: 10; font.weight: Font.Bold; color: window.lang === modelData.toLowerCase() ? "white" : (window.darkMode ? "#888" : "#666") }
                                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: mainWindow.language = modelData.toLowerCase() }
                                    }
                                }
                            }

                            // Theme
                            Label { text: "Theme"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            Switch { text: "Mode sombre"; checked: window.darkMode; onToggled: window.darkMode = checked; leftPadding: 12 }

                            // Horloge
                            Label { text: "Horloge"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            Switch { text: "Afficher l'horloge"; checked: setupController.clockVisible; onToggled: setupController.clockVisible = checked; leftPadding: 12 }
                            RowLayout { spacing: 4; Layout.leftMargin: 12
                                Label { text: "Format:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                                ComboBox {
                                    model: ["HH:mm:ss", "HH:mm", "hh:mm AP"]
                                    currentIndex: {
                                        var f = setupController.clockFormat
                                        if (f === "HH:mm") return 1
                                        if (f === "hh:mm AP") return 2
                                        return 0
                                    }
                                    onCurrentIndexChanged: {
                                        var formats = ["HH:mm:ss", "HH:mm", "hh:mm AP"]
                                        setupController.clockFormat = formats[currentIndex]
                                    }
                                    Layout.fillWidth: true
                                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                }
                            }

                            // Ticker
                            Label { text: "Ticker"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            Switch { text: "Afficher le ticker"; checked: setupController.tickerVisible; onToggled: setupController.tickerVisible = checked; leftPadding: 12 }

                            // Nom chaine
                            Label { text: "Chaine"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            RowLayout { spacing: 4; Layout.leftMargin: 12; Layout.rightMargin: 12
                                TextField { Layout.fillWidth: true; text: configManager.channelName; onTextChanged: configManager.channelName = text; font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"; background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" } }
                            }

                            // Web Remote
                            Label { text: "Controle distant"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            Switch {
                                text: webRemote.running ? ("Actif (:" + webRemote.port + ")") : "Desactive"
                                checked: webRemote.running
                                onToggled: { if (checked) webRemote.start(); else webRemote.stop() }
                                leftPadding: 12
                            }

                            // SDKs
                            Label { text: "SDKs broadcast"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
                            Repeater {
                                model: hardwareScanner.sdkStatus()
                                RowLayout { spacing: 6; Layout.leftMargin: 12
                                    Rectangle { Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: modelData.installed ? "#1DB954" : "#CC3333" }
                                    Label { text: modelData.name; font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333" }
                                }
                            }

                            // Export/Import
                            Rectangle { width: parent.width - 24; height: 1; x: 12; color: window.darkMode ? "#222" : "#CCC" }
                            RowLayout { spacing: 8; Layout.leftMargin: 12
                                Rectangle {
                                    Layout.preferredWidth: 120; Layout.preferredHeight: 32; radius: 6
                                    color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04)
                                    Label { anchors.centerIn: parent; text: "Exporter profil"; font.pixelSize: 10; color: window.darkMode ? "#888" : "#666" }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: exportProfileDialog.open() }
                                }
                                Rectangle {
                                    Layout.preferredWidth: 120; Layout.preferredHeight: 32; radius: 6
                                    color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04)
                                    Label { anchors.centerIn: parent; text: "Importer profil"; font.pixelSize: 10; color: window.darkMode ? "#888" : "#666" }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: importProfileDialog.open() }
                                }
                            }

                            // Copyright
                            Item { implicitHeight: 16 }
                            Label { text: "\u00A9 2024-2026 Prestige Technologie Company"; font.pixelSize: 9; color: window.darkMode ? "#444" : "#AAA"; leftPadding: 12 }
                            Item { implicitHeight: 12 }
                        }
                    }
                }
            }
        }
    }

    // ── Activate overlays confirmation dialog ─────────────────────────
    Dialog {
        id: confirmLaunchDialog
        anchors.centerIn: parent
        width: 480; modal: true
        standardButtons: Dialog.NoButton

        background: Rectangle {
            color: "#12121A"
            radius: 16
            border.color: Qt.rgba(1, 1, 1, 0.06)
            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: Qt.rgba(0, 0, 0, 0.5)
                shadowBlur: 1.0
                shadowVerticalOffset: 8
                shadowHorizontalOffset: 0
            }
        }

        Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.6) }

        header: Item {
            height: 64
            Label {
                anchors.centerIn: parent
                text: window.t("activate_overlays") + " ?"
                font.pixelSize: 20; font.weight: Font.DemiBold
                color: window.darkMode ? "white" : "#1A1A1A"
            }
        }

        contentItem: ColumnLayout {
            spacing: 20

                // Profile info card
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 64; radius: 10
                    color: Qt.rgba(1, 1, 1, 0.04)
                    border.color: Qt.rgba(1, 1, 1, 0.06)
                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 4
                        Label { text: setupController.currentProfileName; font.pixelSize: 15; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A"; Layout.alignment: Qt.AlignHCenter }
                        Label { text: "Style: " + setupController.selectedStyle; font.pixelSize: 12; color: window.darkMode ? "#888" : "#666"; Layout.alignment: Qt.AlignHCenter }
                    }
                }

                Label {
                    text: window.t("activate_overlays_desc")
                    font.pixelSize: 13; color: window.darkMode ? "#777" : "#666"
                    wrapMode: Text.WordWrap; Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                RowLayout {
                    Layout.fillWidth: true; spacing: 12
                    Item { Layout.fillWidth: true }

                    // Cancel button
                    Rectangle {
                        Layout.preferredWidth: 100; Layout.preferredHeight: 40; radius: 8
                        color: mouseCancel.containsMouse ? Qt.rgba(1,1,1,0.08) : Qt.rgba(1,1,1,0.04)
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Label { anchors.centerIn: parent; text: window.t("cancel"); color: window.darkMode ? "#999" : "#666"; font.pixelSize: 14 }
                        MouseArea { id: mouseCancel; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: confirmLaunchDialog.close() }
                    }

                    // Activate button
                    Rectangle {
                        Layout.preferredWidth: 160; Layout.preferredHeight: 40; radius: 8
                        color: mouseLaunch.containsMouse ? "#22C55E" : "#1DB954"
                        Behavior on color { ColorAnimation { duration: 150 } }
                        scale: mouseLaunch.pressed ? 0.97 : 1.0
                        Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }
                        Label { anchors.centerIn: parent; text: window.t("activate_overlays").toUpperCase(); color: "white"; font.pixelSize: 14; font.weight: Font.Bold; font.letterSpacing: 1 }
                        MouseArea { id: mouseLaunch; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: { confirmLaunchDialog.close(); mainWindow.activateOverlays() } }
                    }
                }
            }
    }

    // ── Deactivate overlays confirmation dialog ───────────────────────────
    Dialog {
        id: stopConfirmDialog
        anchors.centerIn: parent; width: 440; modal: true
        standardButtons: Dialog.NoButton

        background: Rectangle {
            color: "#12121A"; radius: 16
            border.color: Qt.rgba(1,1,1,0.06)
            layer.enabled: true
            layer.effect: MultiEffect { shadowEnabled: true; shadowColor: Qt.rgba(0,0,0,0.5); shadowBlur: 1.0; shadowVerticalOffset: 8 }
        }

        Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.6) }

        header: Item {
            height: 64
            Label { anchors.centerIn: parent; text: window.t("deactivate_overlays") + " ?"; font.pixelSize: 20; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" }
        }

        contentItem: ColumnLayout {
            spacing: 20
                Label { text: window.t("deactivate_overlays_desc"); font.pixelSize: 13; color: window.darkMode ? "#999" : "#666"; wrapMode: Text.WordWrap; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        Layout.preferredWidth: 140; Layout.preferredHeight: 40; radius: 8
                        color: msCont.containsMouse ? Qt.rgba(1,1,1,0.08) : Qt.rgba(1,1,1,0.04)
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Label { anchors.centerIn: parent; text: window.t("continue"); color: window.darkMode ? "#999" : "#666"; font.pixelSize: 14 }
                        MouseArea { id: msCont; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: stopConfirmDialog.close() }
                    }
                    Rectangle {
                        Layout.preferredWidth: 160; Layout.preferredHeight: 40; radius: 8
                        color: msStop.containsMouse ? "#DD0000" : "#CC0000"
                        Behavior on color { ColorAnimation { duration: 150 } }
                        scale: msStop.pressed ? 0.97 : 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }
                        Label { anchors.centerIn: parent; text: window.t("deactivate_overlays").toUpperCase(); color: "white"; font.pixelSize: 14; font.weight: Font.Bold; font.letterSpacing: 1 }
                        MouseArea { id: msStop; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: { stopConfirmDialog.close(); mainWindow.deactivateOverlays() } }
                    }
                }
            }
    }

    // ── New Profile Dialog ─────────────────────────────────
    Dialog {
        id: newProfileDialog
        anchors.centerIn: parent; width: 420; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        header: Item { height: 56; Label { anchors.centerIn: parent; text: window.t("new_profile_title"); font.pixelSize: 18; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" } }

        contentItem: ColumnLayout {
            spacing: 16
                Label { text: window.t("emission_name"); font.pixelSize: 12; color: window.darkMode ? "#999" : "#666" }
                TextField {
                    id: newProfileName; Layout.fillWidth: true
                    placeholderText: "Ex: JT 20h -- Lundi"; font.pixelSize: 14; color: window.darkMode ? "white" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        Layout.preferredWidth: 90; Layout.preferredHeight: 36; radius: 6; color: Qt.rgba(1,1,1,0.04)
                        Label { anchors.centerIn: parent; text: window.t("cancel"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 13 }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: newProfileDialog.close() }
                    }
                    Rectangle {
                        Layout.preferredWidth: 100; Layout.preferredHeight: 36; radius: 6; color: "#5B4FDB"
                        Label { anchors.centerIn: parent; text: window.t("create"); color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { setupController.newProfile(newProfileName.text); newProfileDialog.close() } }
                    }
                }
            }
    }

    // ── SDK Status Dialog ──────────────────────────────────
    Dialog {
        id: sdkStatusDialog
        anchors.centerIn: parent; width: 500; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        height: 450

        header: Item { height: 56; Label { anchors.centerIn: parent; text: window.t("sdk_broadcast_status"); font.pixelSize: 18; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" } }

        contentItem: Flickable {
            contentHeight: sdkCol.implicitHeight; clip: true
            flickableDirection: Flickable.VerticalFlick
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            ColumnLayout {
                id: sdkCol; width: parent.width; spacing: 8
                Repeater {
                    model: hardwareScanner.sdkStatus()
                    delegate: Rectangle {
                        width: sdkCol.width; height: 52; radius: 8
                        color: Qt.rgba(1,1,1,0.03); border.color: Qt.rgba(1,1,1,0.05)
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 10
                            Rectangle { Layout.preferredWidth: 10; Layout.preferredHeight: 10; radius: 5; color: modelData.installed ? "#1DB954" : "#CC3333" }
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 1
                                Label { text: modelData.name; font.pixelSize: 13; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" }
                                Label { text: modelData.installed ? window.t("installed") : modelData.installHint; font.pixelSize: 11; color: modelData.installed ? "#1DB954" : (window.darkMode ? "#888" : "#666"); elide: Text.ElideRight; Layout.fillWidth: true }
                            }
                        }
                    }
                }
                Item { Layout.preferredHeight: 8 }
                Rectangle {
                    Layout.preferredWidth: 80; Layout.preferredHeight: 32; radius: 6; color: Qt.rgba(1,1,1,0.04)
                    Layout.alignment: Qt.AlignRight
                    Label { anchors.centerIn: parent; text: window.t("close"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 12 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: sdkStatusDialog.close() }
                }
            }
        }
    }

    // ── Shortcuts Dialog ───────────────────────────────────
    Dialog {
        id: shortcutsDialog
        anchors.centerIn: parent; width: 420; modal: true
        height: 520
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        header: Item { height: 56; Label { anchors.centerIn: parent; text: window.t("shortcuts_title"); font.pixelSize: 18; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" } }

        contentItem: Flickable {
            contentHeight: shortcutsCol.implicitHeight; clip: true
            flickableDirection: Flickable.VerticalFlick
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            ColumnLayout {
                id: shortcutsCol; width: parent.width; spacing: 6
                Repeater {
                    model: [
                        { key: "Ctrl+N", action: "Nouveau profil" },
                        { key: "Ctrl+S", action: "Sauvegarder le profil" },
                        { key: "F5",     action: "Activer / Desactiver overlays" },
                        { key: "Ctrl+T", action: "Gestion des talents" },
                        { key: "Ctrl+1", action: "Ouvrir panneau" },
                        { key: "Ctrl+2", action: "Toggle overlays" },
                        { key: "F11",    action: "Plein ecran" },
                        { key: "Ctrl+Q", action: "Quitter" },
                        { key: "Space",  action: "Toggle overlays (live)" },
                        { key: "R",      action: "Toggle enregistrement (live)" },
                        { key: "B",      action: "Toggle bypass (live)" }
                    ]
                    delegate: RowLayout {
                        width: shortcutsCol.width; spacing: 12
                        Rectangle {
                            Layout.preferredWidth: 80; Layout.preferredHeight: 26; radius: 4; color: Qt.rgba(1,1,1,0.06)
                            Label { anchors.centerIn: parent; text: modelData.key; font.pixelSize: 11; font.family: "Menlo"; color: window.darkMode ? "#AAA" : "#555" }
                        }
                        Label { text: modelData.action; font.pixelSize: 13; color: window.darkMode ? "#CCC" : "#333" }
                    }
                }
                Item { Layout.preferredHeight: 8 }
                Rectangle {
                    Layout.preferredWidth: 80; Layout.preferredHeight: 32; radius: 6; color: Qt.rgba(1,1,1,0.04)
                    Layout.alignment: Qt.AlignRight
                    Label { anchors.centerIn: parent; text: window.t("close"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 12 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: shortcutsDialog.close() }
                }
            }
        }
    }

    // ── About Dialog ───────────────────────────────────────
    Dialog {
        id: aboutDialog
        anchors.centerIn: parent; width: 380; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        contentItem: ColumnLayout {
            spacing: 12
                Item { Layout.preferredHeight: 8 }
                Label { text: "PRESTIGE AI"; font.pixelSize: 24; font.weight: Font.Bold; font.letterSpacing: 3; color: "#5B4FDB"; Layout.alignment: Qt.AlignHCenter }
                Label { text: "v1.0.0"; font.pixelSize: 14; color: window.darkMode ? "#888" : "#666"; Layout.alignment: Qt.AlignHCenter }
                Item { Layout.preferredHeight: 4 }
                Label { text: "Logiciel broadcast professionnel\navec reconnaissance faciale IA"; font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#555"; horizontalAlignment: Text.AlignHCenter; Layout.alignment: Qt.AlignHCenter; wrapMode: Text.WordWrap }
                Item { Layout.preferredHeight: 4 }
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Qt.rgba(1,1,1,0.06) }
                Label { text: "\u00A9 2024-2026 Prestige Technologie Company"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#CCC" : "#333"; Layout.alignment: Qt.AlignHCenter }
                Label { text: "Tous droits reserves"; font.pixelSize: 10; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter }
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Qt.rgba(1,1,1,0.06) }
                Item { Layout.preferredHeight: 2 }
                Label { text: "20 styles d'overlay \u2022 8 animations"; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter }
                Label { text: "InsightFace \u2022 FFmpeg \u2022 ZeroMQ \u2022 Qt 6"; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter }
                Label { text: "DeckLink \u2022 AJA \u2022 NDI \u2022 Magewell (runtime)"; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter }
                Item { Layout.preferredHeight: 4 }
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Qt.rgba(1,1,1,0.06) }
                Label { text: "Licence: " + licenseManager.licenseType; font.pixelSize: 11; color: "#1DB954"; Layout.alignment: Qt.AlignHCenter }
                Label { text: "Expire: " + licenseManager.expirationDate; font.pixelSize: 10; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter }
                Item { Layout.preferredHeight: 4 }
                Rectangle {
                    Layout.preferredWidth: 80; Layout.preferredHeight: 32; radius: 6; color: Qt.rgba(1,1,1,0.04)
                    Layout.alignment: Qt.AlignHCenter
                    Label { anchors.centerIn: parent; text: window.t("close"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 12 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: aboutDialog.close() }
                }
            }
    }

    // ── Toast notifications ────────────────────────────────
    Rectangle {
        id: savedNotif
        anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 80
        width: notifLbl.implicitWidth + 32; height: 36; radius: 18
        color: Qt.rgba(29/255, 185/255, 84/255, 0.9)
        opacity: 0; visible: opacity > 0
        function show() { opacity = 1; notifHideTimer.start() }
        Behavior on opacity { NumberAnimation { duration: 300 } }
        Label { id: notifLbl; anchors.centerIn: parent; text: "\u2713  " + window.t("profile_saved"); color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
        Timer { id: notifHideTimer; interval: 2000; onTriggered: savedNotif.opacity = 0 }
    }

    Rectangle {
        id: hardwareScanNotif
        anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 80
        width: hwLbl.implicitWidth + 32; height: 36; radius: 18
        color: Qt.rgba(91/255, 79/255, 219/255, 0.9)
        opacity: 0; visible: opacity > 0
        function show() { opacity = 1; hwHideTimer.start() }
        Behavior on opacity { NumberAnimation { duration: 300 } }
        Label { id: hwLbl; anchors.centerIn: parent; text: "\u2713  " + window.t("hw_scan_done"); color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
        Timer { id: hwHideTimer; interval: 2000; onTriggered: hardwareScanNotif.opacity = 0 }
    }

    // ── Tool dialogs ───────────────────────────────────
    Dialog {
        id: teleprompterDialog; anchors.centerIn: parent; width: 500; height: 500; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }
        contentItem: TeleprompterPanel {}
    }
    Dialog {
        id: macroDialog; anchors.centerIn: parent; width: 460; height: 480; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }
        contentItem: MacroPanel {}
    }
    Dialog {
        id: statsDialog; anchors.centerIn: parent; width: 420; height: 400; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }
        contentItem: StatsPanel {}
    }
    Dialog {
        id: tickerDialog; anchors.centerIn: parent; width: 460; height: 400; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }
        contentItem: TickerPanel {}
    }

    // ── Export Profile Dialog (Feature 10) ─────────────────
    Dialog {
        id: exportProfileDialog
        anchors.centerIn: parent; width: 460; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        header: Item { height: 56; Label { anchors.centerIn: parent; text: window.t("export_title"); font.pixelSize: 18; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" } }

        contentItem: ColumnLayout {
            spacing: 16
                Label { text: window.t("file_path"); font.pixelSize: 12; color: window.darkMode ? "#999" : "#666" }
                TextField {
                    id: exportPathField; Layout.fillWidth: true
                    text: setupController.currentProfileName + ".json"
                    font.pixelSize: 14; color: window.darkMode ? "white" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        Layout.preferredWidth: 90; Layout.preferredHeight: 36; radius: 6; color: Qt.rgba(1,1,1,0.04)
                        Label { anchors.centerIn: parent; text: window.t("cancel"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 13 }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: exportProfileDialog.close() }
                    }
                    Rectangle {
                        Layout.preferredWidth: 110; Layout.preferredHeight: 36; radius: 6; color: "#5B4FDB"
                        Label { anchors.centerIn: parent; text: window.t("export"); color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { setupController.exportProfile(exportPathField.text); exportProfileDialog.close() } }
                    }
                }
            }
    }

    // ── Import Profile Dialog (Feature 10) ─────────────────
    Dialog {
        id: importProfileDialog
        anchors.centerIn: parent; width: 460; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        header: Item { height: 56; Label { anchors.centerIn: parent; text: window.t("import_title"); font.pixelSize: 18; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" } }

        contentItem: ColumnLayout {
            spacing: 16
                Label { text: window.t("json_path"); font.pixelSize: 12; color: window.darkMode ? "#999" : "#666" }
                TextField {
                    id: importPathField; Layout.fillWidth: true
                    placeholderText: "/chemin/vers/profil.json"
                    font.pixelSize: 14; color: window.darkMode ? "white" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        Layout.preferredWidth: 90; Layout.preferredHeight: 36; radius: 6; color: Qt.rgba(1,1,1,0.04)
                        Label { anchors.centerIn: parent; text: window.t("cancel"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 13 }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: importProfileDialog.close() }
                    }
                    Rectangle {
                        Layout.preferredWidth: 110; Layout.preferredHeight: 36; radius: 6; color: "#5B4FDB"
                        Label { anchors.centerIn: parent; text: window.t("import"); color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { setupController.importProfile(importPathField.text); importProfileDialog.close() } }
                    }
                }
            }
    }

    // ── License Activation Dialog ────────────────────────────
    Dialog {
        id: licenseDialog
        anchors.centerIn: parent; width: 520; modal: true
        standardButtons: Dialog.NoButton
        closePolicy: Dialog.NoAutoClose

        Component.onCompleted: {
            if (!licenseManager.isActivated && !licenseManager.hasStoredLicense())
                licenseDialog.open()
        }

        Connections {
            target: licenseManager
            function onActivationSuccess() { licenseDialog.close() }
            function onStatusChanged() {
                if (licenseManager.isActivated) licenseDialog.close()
            }
        }

        background: Rectangle { color: "#0A0A12"; radius: 16; border.color: Qt.rgba(91/255,79/255,219/255,0.3) }
        Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.85) }

        contentItem: ColumnLayout {
            spacing: 16
            Item { Layout.preferredHeight: 16 }

            Label { text: "PRESTIGE AI"; font.pixelSize: 32; font.weight: Font.Bold; font.letterSpacing: 5; color: "#5B4FDB"; Layout.alignment: Qt.AlignHCenter }
            Label { text: "Broadcast Overlay System"; font.pixelSize: 13; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter }
            Label { text: "\u00A9 Prestige Technologie Company"; font.pixelSize: 11; color: window.darkMode ? "#555" : "#999"; Layout.alignment: Qt.AlignHCenter }

            Item { Layout.preferredHeight: 8 }

            Rectangle {
                Layout.fillWidth: true; Layout.leftMargin: 24; Layout.rightMargin: 24
                Layout.preferredHeight: activationCol.implicitHeight + 32; radius: 10
                color: Qt.rgba(1,1,1,0.03); border.color: Qt.rgba(1,1,1,0.06)

                ColumnLayout {
                    id: activationCol
                    anchors.fill: parent; anchors.margins: 16; spacing: 12

                    Label { text: "Activation de la licence"; font.pixelSize: 16; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A"; Layout.alignment: Qt.AlignHCenter }
                    Label { text: "Entrez votre cle de licence pour activer le logiciel"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#666"; Layout.alignment: Qt.AlignHCenter }

                    TextField {
                        id: licenseKeyInput
                        Layout.fillWidth: true
                        placeholderText: "XXXX-XXXX-XXXX-XXXX"
                        font.pixelSize: 16; font.family: "Menlo"; color: window.darkMode ? "white" : "#1A1A1A"
                        horizontalAlignment: Text.AlignHCenter
                        maximumLength: 19
                        background: Rectangle { color: "#1A1A22"; radius: 8; border.color: licenseKeyInput.text.length > 0 ? "#5B4FDB" : "#333"; border.width: licenseKeyInput.text.length > 0 ? 2 : 1 }
                    }

                    Rectangle {
                        Layout.preferredWidth: 200; Layout.preferredHeight: 44; radius: 10
                        Layout.alignment: Qt.AlignHCenter
                        color: {
                            if (licenseManager.isChecking) return "#333"
                            if (licenseKeyInput.text.trim().length >= 10) return activateMA.containsMouse ? "#6B5FEB" : "#5B4FDB"
                            return Qt.rgba(1,1,1,0.06)
                        }

                        Label {
                            anchors.centerIn: parent
                            text: licenseManager.isChecking ? "Verification..." : "ACTIVER"
                            color: licenseKeyInput.text.trim().length >= 10 ? "white" : "#555"
                            font.pixelSize: 14; font.weight: Font.Bold; font.letterSpacing: 1
                        }
                        MouseArea {
                            id: activateMA; anchors.fill: parent; hoverEnabled: true
                            cursorShape: !licenseManager.isChecking && licenseKeyInput.text.trim().length >= 10 ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: {
                                if (!licenseManager.isChecking && licenseKeyInput.text.trim().length >= 10)
                                    licenseManager.activateKey(licenseKeyInput.text.trim())
                            }
                        }
                    }

                    Label {
                        visible: licenseManager.errorString !== ""
                        text: licenseManager.errorString
                        font.pixelSize: 11; color: "#CC3333"
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Label { text: "Prestige Technologie Company"; font.pixelSize: 10; color: window.darkMode ? "#444" : "#AAA"; Layout.alignment: Qt.AlignHCenter }

            Item { Layout.preferredHeight: 12 }
        }
    }

    // ── Welcome Dialog (first launch -- channel name) ──────
    Dialog {
        id: welcomeDialog
        anchors.centerIn: parent; width: 520; modal: true
        standardButtons: Dialog.NoButton
        closePolicy: Dialog.NoAutoClose

        Component.onCompleted: {
            if (licenseManager.isActivated && configManager.channelName === "")
                welcomeDialog.open()
        }

        Connections {
            target: licenseManager
            function onActivationSuccess() {
                if (configManager.channelName === "")
                    welcomeDialog.open()
            }
            function onStatusChanged() {
                if (licenseManager.isActivated && configManager.channelName === "" && !licenseDialog.visible)
                    welcomeDialog.open()
            }
        }

        background: Rectangle { color: "#12121A"; radius: 16; border.color: Qt.rgba(1,1,1,0.06) }
        Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.8) }

        contentItem: ColumnLayout {
            spacing: 16
                Item { Layout.preferredHeight: 12 }
                Label { text: "PRESTIGE AI"; font.pixelSize: 28; font.weight: Font.Bold; font.letterSpacing: 4; color: "#5B4FDB"; Layout.alignment: Qt.AlignHCenter }
                Label { text: window.t("welcome"); font.pixelSize: 18; color: window.darkMode ? "white" : "#1A1A1A"; Layout.alignment: Qt.AlignHCenter }
                Item { Layout.preferredHeight: 8 }
                Label { text: window.t("enter_channel"); font.pixelSize: 13; color: window.darkMode ? "#999" : "#666"; Layout.alignment: Qt.AlignHCenter }
                Label { text: window.t("channel_overlay_desc"); font.pixelSize: 11; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter }
                Item { Layout.preferredHeight: 4 }
                TextField {
                    id: channelNameInput
                    Layout.fillWidth: true; Layout.leftMargin: 24; Layout.rightMargin: 24
                    placeholderText: "Ex: Africa 24, MediTV, Canal Prestige..."
                    font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A"
                    horizontalAlignment: Text.AlignHCenter
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 8; border.color: channelNameInput.text.length > 0 ? "#5B4FDB" : (window.darkMode ? "#333" : "#CCC"); border.width: channelNameInput.text.length > 0 ? 2 : 1 }
                }
                Item { Layout.preferredHeight: 8 }
                Rectangle {
                    Layout.preferredWidth: 200; Layout.preferredHeight: 44; radius: 10
                    Layout.alignment: Qt.AlignHCenter
                    color: channelNameInput.text.trim().length > 0 ? "#5B4FDB" : Qt.rgba(1,1,1,0.06)
                    enabled: channelNameInput.text.trim().length > 0
                    Label { anchors.centerIn: parent; text: window.t("start"); color: channelNameInput.text.trim().length > 0 ? "white" : "#555"; font.pixelSize: 14; font.weight: Font.Bold; font.letterSpacing: 1 }
                    MouseArea {
                        anchors.fill: parent; cursorShape: channelNameInput.text.trim().length > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: {
                            if (channelNameInput.text.trim().length > 0) {
                                configManager.channelName = channelNameInput.text.trim()
                                welcomeDialog.close()
                            }
                        }
                    }
                }
                Item { Layout.preferredHeight: 8 }
                Label { text: "\u00A9 2024-2026 Prestige Technologie Company"; font.pixelSize: 10; color: "#444"; Layout.alignment: Qt.AlignHCenter }
                Item { Layout.preferredHeight: 8 }
            }
    }

    // ── Channel Name Edit Dialog ──────────────────────────
    Dialog {
        id: channelNameDialog
        anchors.centerIn: parent; width: 460; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        onOpened: channelNameEditField.text = configManager.channelName

        header: Item { height: 56; Label { anchors.centerIn: parent; text: window.t("channel_name_title"); font.pixelSize: 18; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" } }

        contentItem: ColumnLayout {
            spacing: 16
                Label { text: window.t("channel_name_desc"); font.pixelSize: 12; color: window.darkMode ? "#999" : "#666" }
                TextField {
                    id: channelNameEditField; Layout.fillWidth: true
                    placeholderText: "Ex: Africa 24, MediTV, Canal Prestige..."
                    font.pixelSize: 14; color: window.darkMode ? "white" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        Layout.preferredWidth: 90; Layout.preferredHeight: 36; radius: 6; color: Qt.rgba(1,1,1,0.04)
                        Label { anchors.centerIn: parent; text: window.t("cancel"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 13 }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: channelNameDialog.close() }
                    }
                    Rectangle {
                        Layout.preferredWidth: 110; Layout.preferredHeight: 36; radius: 6; color: "#5B4FDB"
                        Label { anchors.centerIn: parent; text: window.t("register"); color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { configManager.channelName = channelNameEditField.text.trim(); channelNameDialog.close() } }
                    }
                }
            }
    }

    // ── Logo File Dialog ────────────────────────────────────
    Loader {
        id: logoFileDialogLoader
        active: false
        sourceComponent: Component {
            Item {
                id: dialogHost
                Component.onCompleted: {
                    var dlg = Qt.createQmlObject('
                        import QtQuick 2.15;
                        import Qt.labs.platform 1.1 as Platform;
                        Platform.FileDialog {
                            id: innerDlg;
                            title: "Choisir un logo";
                            nameFilters: ["Images (*.png *.jpg *.jpeg *.gif *.bmp *.svg)"];
                            onAccepted: { setupController.channelLogoPath = file.toString(); logoFileDialogLoader.active = false; }
                            onRejected: { logoFileDialogLoader.active = false; }
                            Component.onCompleted: open()
                        }', dialogHost, "logoFileDialog");
                }
            }
        }
    }

    // ── License Info Dialog ────────────────────────────────
    Dialog {
        id: licenseInfoDialog
        anchors.centerIn: parent; width: 420; modal: true
        standardButtons: Dialog.NoButton
        background: Rectangle { color: window.darkMode ? "#16161A" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.1) }
        Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.6) }

        contentItem: ColumnLayout {
            spacing: 12
            Item { Layout.preferredHeight: 8 }
            Label { text: "LICENCE"; font.pixelSize: 20; font.weight: Font.Bold; font.letterSpacing: 3; color: "#5B4FDB"; Layout.alignment: Qt.AlignHCenter }
            Label { text: "\u00A9 Prestige Technologie Company"; font.pixelSize: 11; color: window.darkMode ? "#555" : "#999"; Layout.alignment: Qt.AlignHCenter }
            Item { Layout.preferredHeight: 4 }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Qt.rgba(1,1,1,0.06) }
            Label { text: "Statut: " + (licenseManager.isActivated ? "Active" : "Inactive"); font.pixelSize: 13; color: licenseManager.isActivated ? "#1DB954" : "#CC3333"; Layout.alignment: Qt.AlignHCenter }
            Label { text: "Type: " + licenseManager.licenseType; font.pixelSize: 12; color: window.darkMode ? "#AAA" : "#555"; Layout.alignment: Qt.AlignHCenter; visible: licenseManager.licenseType !== "" }
            Label { text: "Cle: " + licenseManager.licenseKey; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter; visible: licenseManager.licenseKey !== "" }
            Label { text: "Expiration: " + licenseManager.expirationDate; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999"; Layout.alignment: Qt.AlignHCenter; visible: licenseManager.expirationDate !== "" }
            Item { Layout.preferredHeight: 8 }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter; spacing: 12
                Rectangle {
                    Layout.preferredWidth: 110; Layout.preferredHeight: 36; radius: 6; color: "#CC3333"; visible: licenseManager.isActivated
                    Label { anchors.centerIn: parent; text: "Desactiver"; color: "white"; font.pixelSize: 12; font.weight: Font.DemiBold }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { licenseManager.deactivate(); licenseInfoDialog.close() } }
                }
                Rectangle {
                    Layout.preferredWidth: 80; Layout.preferredHeight: 36; radius: 6; color: Qt.rgba(1,1,1,0.04)
                    Label { anchors.centerIn: parent; text: window.t("close"); color: window.darkMode ? "#888" : "#666"; font.pixelSize: 12 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: licenseInfoDialog.close() }
                }
            }
            Item { Layout.preferredHeight: 8 }
        }
    }
}
