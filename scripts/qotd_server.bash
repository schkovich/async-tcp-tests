#!/usr/bin/env bash

# qotd_server.bash — [RFC-865](https://datatracker.ietf.org/doc/html/rfc865)
# compliant QOTD line from quotable.io
# There is a rate limit of 180 requests per minute, per IP address.
# If exceeded, the API will respond with a 429 error.
# https://github.com/lukePeavey/quotable
# To emulate QOTD server run:
# ncat -l 17 --keep-open --send-only --exec "./scripts/qotd_server.bash"

set -euo pipefail

# Fetch JSON (uses --insecure only because your system CA bundle is out of date)
json="$(curl -s --insecure --max-time 5 https://api.quotable.io/random || true)"

# If fetch failed, emit a safe fallback
if [ -z "${json:-}" ]; then
  # RFC-865 multi-line fallback: use real newlines and CRLF
  line="First line of the quote.
Second line of the quote.
Third line with CRLF."
else
  # Requires jq. Extract plain fields (they are UTF-8).
  author="$(printf '%s' "$json" | jq -r '.author // empty' || true)"
  content="$(printf '%s' "$json" | jq -r '.content // empty' || true)"

  # Fallbacks in case fields are missing
  if [ -z "$content" ]; then content="Keep it simple."; fi
  if [ -z "$author" ]; then author="Unknown"; fi

  # Build single line in UTF-8 (use an ASCII dash by design for compliance)
  line="$(printf '%s - %s' "$content" "$author")"
fi

# --- RFC-865 compliance filters ---
# 1) Convert to ASCII (transliterate where possible), drop anything non-ASCII.
# 2) Strip control chars except CR/LF, collapse whitespace to single spaces (except line breaks).
# 3) Trim ends of each line.
ascii_line="$(
  printf '%s' "$line" \
  | iconv -f UTF-8 -t ASCII//TRANSLIT 2>/dev/null \
  | tr -d '\000-\011\013\014\016-\037\177' \
  | sed -E ':a;N;$!ba;s/[ \t]+/ /g' \
  | sed -E 's/^ +//; s/ +$//; s/^[\r\n]+//; s/[\r\n]+$//'
)"

# Ensure not empty after sanitization
if [ -z "$ascii_line" ]; then
  ascii_line="Keep it simple. - Unknown"
fi

# 4) Enforce 512-char max (conservative: counting all bytes including CR/LF)
ascii_line="$(printf '%s' "$ascii_line" | head -c 512)"

printf '%s\n' "$ascii_line"
