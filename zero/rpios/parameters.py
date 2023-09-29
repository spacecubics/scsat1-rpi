import json


def list_to_tuple_recursive(data):
    if isinstance(data, list):
        return tuple(list_to_tuple_recursive(item) for item in data)
    elif isinstance(data, dict):
        return {key: list_to_tuple_recursive(value) for key, value in data.items()}
    else:
        return data


def load_metadata_from_json_with_tuple(json_path):
    try:
        with open(json_path, 'r') as f:
            metadata = json.load(f)
        return list_to_tuple_recursive(metadata)
    except FileNotFoundError:
        print(f"File {json_path} not found.")
        return None
    except json.JSONDecodeError:
        print(f"Error decoding JSON from {json_path}.")
        return None


def refresh_metadata(metadata: dict):
    metadata = load_metadata_from_json_with_tuple('./metadata.json')


# 関数が呼び出されるたびに.jsonファイルを読み込む
def check_metadata(metadata: dict):
    print("-------------------------------")
    for key, value in metadata.items():
        print(f'{key}: {value}')
    print("-------------------------------")


def adjust_metadata(metadata: dict, key, new_value):
    if key in metadata:
        metadata[key] = new_value
        print(f"Adjust {key} to {new_value}")
    else:
        print(f"Key {key} not found in metadata")


# ToDo: 入力されたデータ型や要素数が異なれば、エラーを出力する機能を追加
# Windowsのパス指定方法だとモジュールで呼び出した際にエラーが生じる
def update_metadata(json_path, key, new_value):
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            metadata = json.load(f)
    except FileNotFoundError:
        print(f"File {json_path} not found.")
        return
    except json.JSONDecodeError:
        print(f"Error decoding JSON from {json_path}.")
        return

    if key in metadata:
        metadata[key] = new_value
        print(f"Updated {key} to {new_value}")
    else:
        print(f"Key {key} not found in metadata.")

    with open(json_path, 'w') as f:
        json.dump(metadata, f, indent=4)


def describe_metadata(metadata: dict):
    description = {}
    for key, value in metadata.items():
        description[key] = {'value': value, 'type': type(value).__name__}
        print(f"{key}:\n  Value: {value}\n  Type: {type(value).__name__}\n")


metadata = load_metadata_from_json_with_tuple('./metadata.json')

if __name__ == "__main__":
    print(describe_metadata(metadata))
    check_metadata(metadata)
    adjust_metadata(metadata, 'ColourTemperature', 5000)
    update_metadata('Product\metadata.json', 'ColourTemperature', 4500)
    check_metadata(metadata)
