#!/usr/bin/env python3
# ============================================================
# Prestige AI -- Social Chat Fetcher (YouTube / Twitch)
#
# Architecture: fetches chat messages from YouTube Live Chat
# or Twitch IRC, publishes via ZMQ multipart on port 5555.
#
# Activate by providing credentials and calling start().
# ============================================================

import json
import time
import threading
import socket
import ssl

try:
    import zmq
except ImportError:
    zmq = None


class SocialChatPublisher:
    """Publishes social chat messages via ZMQ multipart [b"social_chat", JSON]."""

    def __init__(self, port=5555):
        self._port = port
        self._ctx = None
        self._sock = None

    def start(self):
        if zmq is None:
            print("[SocialChat] pyzmq not installed -- publisher disabled")
            return
        self._ctx = zmq.Context()
        self._sock = self._ctx.socket(zmq.PUB)
        self._sock.connect(f"tcp://127.0.0.1:{self._port}")
        print(f"[SocialChat] Publisher connected to :{self._port}")

    def publish(self, platform: str, author: str, message: str,
                color: str = "#FFFFFF", timestamp_ms: int = 0):
        if self._sock is None:
            return
        if timestamp_ms == 0:
            timestamp_ms = int(time.time() * 1000)
        payload = json.dumps({
            "type": "social_chat",
            "platform": platform,
            "author": author,
            "message": message,
            "color": color,
            "timestamp_ms": timestamp_ms,
        })
        self._sock.send_multipart([b"social_chat", payload.encode("utf-8")])

    def stop(self):
        if self._sock:
            self._sock.close()
        if self._ctx:
            self._ctx.term()


class TwitchChatFetcher:
    """Connects to Twitch IRC and reads chat messages.

    Usage:
        fetcher = TwitchChatFetcher(channel="channelname",
                                     oauth_token="oauth:xxx",
                                     nickname="botname")
        fetcher.start(publisher)
    """

    IRC_HOST = "irc.chat.twitch.tv"
    IRC_PORT = 6697  # TLS port (plaintext 6667 is deprecated by Twitch)
    MAX_RECONNECT_DELAY = 30

    def __init__(self, channel: str = "", oauth_token: str = "", nickname: str = "prestige_bot"):
        self._channel = channel
        self._token = oauth_token
        self._nick = nickname
        self._running = False
        self._thread = None

    def start(self, publisher: SocialChatPublisher):
        if not self._channel or not self._token:
            print("[TwitchChat] No channel/token configured -- stub mode")
            return
        self._running = True
        self._thread = threading.Thread(target=self._run, args=(publisher,), daemon=True)
        self._thread.start()

    def stop(self):
        self._running = False
        if self._thread:
            self._thread.join(timeout=3)

    def _connect_tls(self) -> socket.socket:
        """Create TLS-encrypted connection to Twitch IRC (SMPTE/EBU compliant)."""
        raw = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ctx = ssl.create_default_context()
        sock = ctx.wrap_socket(raw, server_hostname=self.IRC_HOST)
        sock.connect((self.IRC_HOST, self.IRC_PORT))
        sock.settimeout(2.0)

        sock.sendall(f"PASS {self._token}\r\n".encode("utf-8"))
        sock.sendall(f"NICK {self._nick}\r\n".encode("utf-8"))
        # Request tags for display names, badges, colors
        sock.sendall(b"CAP REQ :twitch.tv/tags twitch.tv/commands\r\n")
        sock.sendall(f"JOIN #{self._channel}\r\n".encode("utf-8"))
        return sock

    def _run(self, publisher: SocialChatPublisher):
        reconnect_delay = 1
        while self._running:
            try:
                sock = self._connect_tls()
                reconnect_delay = 1  # Reset on successful connect
                print(f"[TwitchChat] Connected to #{self._channel} (TLS)")

                buffer = ""
                while self._running:
                    try:
                        data = sock.recv(4096).decode("utf-8", errors="replace")
                        if not data:
                            break  # Connection lost → reconnect
                        buffer += data
                        while "\r\n" in buffer:
                            line, buffer = buffer.split("\r\n", 1)
                            if line.startswith("PING"):
                                sock.sendall(f"PONG {line[5:]}\r\n".encode("utf-8"))
                            elif "PRIVMSG" in line:
                                self._parse_and_publish(line, publisher)
                    except socket.timeout:
                        continue
                sock.close()
            except Exception as e:
                print(f"[TwitchChat] Error: {e} — reconnecting in {reconnect_delay}s")

            if not self._running:
                break
            # Exponential backoff reconnection
            time.sleep(reconnect_delay)
            reconnect_delay = min(reconnect_delay * 2, self.MAX_RECONNECT_DELAY)

    def _parse_and_publish(self, line: str, publisher: SocialChatPublisher):
        # Format: :nickname!user@host PRIVMSG #channel :message
        try:
            prefix, _, rest = line.partition(" PRIVMSG ")
            author = prefix.split("!")[0].lstrip(":")
            _, _, message = rest.partition(":")
            publisher.publish("twitch", author, message, "#9147FF")
        except Exception:
            pass


class YouTubeChatFetcher:
    """Stub for YouTube Live Chat polling.

    To activate, provide a YouTube Live Chat ID and API key.
    This stub architecture is ready for pytchat or YouTube Data API v3.
    """

    def __init__(self, video_id: str = "", api_key: str = ""):
        self._video_id = video_id
        self._api_key = api_key
        self._running = False
        self._thread = None

    def start(self, publisher: SocialChatPublisher):
        if not self._video_id:
            print("[YouTubeChat] No video_id configured -- stub mode")
            return
        self._running = True
        self._thread = threading.Thread(target=self._run, args=(publisher,), daemon=True)
        self._thread.start()

    def stop(self):
        self._running = False
        if self._thread:
            self._thread.join(timeout=3)

    def _run(self, publisher: SocialChatPublisher):
        """Stub polling loop. Replace with pytchat or YouTube API calls."""
        print(f"[YouTubeChat] Fetcher started for video: {self._video_id}")
        while self._running:
            # TODO: Implement actual YouTube chat polling here
            # Example with pytchat:
            #   import pytchat
            #   chat = pytchat.create(video_id=self._video_id)
            #   while chat.is_alive() and self._running:
            #       for msg in chat.get().sync_items():
            #           publisher.publish("youtube", msg.author.name, msg.message, "#FF0000")
            time.sleep(2)


# ── Standalone test ─────────────────────────────────────────
if __name__ == "__main__":
    pub = SocialChatPublisher()
    pub.start()

    # Demo: publish a test message every 3 seconds
    import itertools
    names = itertools.cycle(["Alice", "Bob", "Charlie", "Diana"])
    msgs = itertools.cycle(["Bravo!", "Super emission!", "Merci", "Tres bien"])
    platforms = itertools.cycle(["youtube", "twitch"])
    try:
        while True:
            pub.publish(next(platforms), next(names), next(msgs), "#5B4FDB")
            time.sleep(3)
    except KeyboardInterrupt:
        pub.stop()
