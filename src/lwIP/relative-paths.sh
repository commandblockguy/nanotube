echo "$1"
char="/"
count=$(echo "$1" | awk -F"${char}" '{print NF-1}')
parents="$((count - 1))"
prefix="$(
  for ((var = 0; var < parents; ++var)); do echo -n "\\.\\.\\/"; done
  echo
)include\\/"
sed -E "s/#include \"(lwip|compat|netif)/#include \"${prefix}\\1/g" -i "$1"
