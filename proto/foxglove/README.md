# foxglove schemas (proto)

All `.proto` files in this directory are pulled directly from
[foxglove/schemas](https://github.com/foxglove/schemas) and **must not be
hand-edited**.

## Updating

```bash
# Pull all foxglove proto schemas from the official repo
cd proto/foxglove
BASE_URL="https://raw.githubusercontent.com/foxglove/schemas/main/schemas/proto/foxglove"
API_URL="https://api.github.com/repos/foxglove/schemas/contents/schemas/proto/foxglove"

for f in $(curl -fsSL "$API_URL" | jq -r '.[] | select(.name | endswith(".proto")) | .name'); do
  curl -fsSL -o "$f" "${BASE_URL}/${f}"
done
```

After updating, regenerate code:

```bash
buf generate
```
