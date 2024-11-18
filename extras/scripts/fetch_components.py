import logging
import os
import sys
import subprocess
import shutil
import json
import argparse
import re as regex


logging.basicConfig(level=logging.INFO)
logging.getLogger().handlers[0].setFormatter(
    logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
)


def git_clone(config: dict, output_dir: str):
    url = config["url"]
    branch = config.get("branch", "main")
    depth = config.get("depth", 1)

    if os.path.exists(output_dir):
        logging.info("removing existing directory: %s", output_dir)
        shutil.rmtree(output_dir)

    logging.info("cloning %s to %s", url, output_dir)
    p = subprocess.run(
        ["git", "clone", "--depth", str(depth), "--branch", branch, url, output_dir],
        check=True,
    )
    if p.returncode != 0:
        raise Exception(f"Failed to clone {url}")


def synchronize_files(src_dir: str, dst_dir: str, includes: list, excludes: list):
    logging.info("synchronizing files from %s to %s", src_dir, dst_dir)

    src_files = []
    for root, _, files in os.walk(src_dir):
        for file in files:
            src_files.append(os.path.relpath(os.path.join(root, file), src_dir))

    includes_files = []
    for file in src_files:
        for regex_str in includes:
            if regex.match(regex_str, file):
                includes_files.append(file)
                break

    keep_files = []
    print(excludes)
    for file in includes_files:
        found = False
        for regex_str in excludes:
            if regex.match(regex_str, file):
                found = True
                break
        if not found:
            keep_files.append(file)
        
    logging.info("copying %d files from %s to %s", len(keep_files), src_dir, dst_dir)
    os.makedirs(dst_dir, exist_ok=True)
    dst_files = []
    for root, _, files in os.walk(dst_dir):
        for file in files:
            dst_files.append(os.path.relpath(os.path.join(root, file), dst_dir))
    for file in keep_files:
        src_path = os.path.join(src_dir, file)
        dst_path = os.path.join(dst_dir, file)
        os.makedirs(os.path.dirname(dst_path), exist_ok=True)
        shutil.copyfile(src_path, dst_path)

    orphan_files = set(dst_files) - set(keep_files)
    logging.info("removing %d orphan files from %s", len(orphan_files), dst_dir)
    for file in orphan_files:
        os.remove(os.path.join(dst_dir, file))
    
    for root, dirs, _ in os.walk(dst_dir, topdown=False):
        for dir in dirs:
            dir_path = os.path.join(root, dir)
            if not os.listdir(dir_path):
                os.rmdir(dir_path)
    

def apply_modifiers(src_dir: str, modifiers: list):
    logging.info("applying %d modifiers to %s", len(modifiers), src_dir)

    def op_insert_if_not_exist(lines, at: str, data: str):
        logging.debug("inserting %s at %s", data, at)

        at_index = -1
        for i, line in enumerate(lines):
            line = line.strip()
            if data == line:
                logging.warning("data already exists: %s", data)
                return
            if at == line:
                at_index = i + 1
        if at_index == -1:
            logging.warning("target not found: %s", at)
            return

        if not data.endswith('\n'):
            data += '\n'
        lines.insert(at_index, data)

    def op_replace_if_exist(lines, at: str, data: str):
        logging.debug("replacing %s at %s", data, at)

        at_index = -1
        for i, line in enumerate(lines):
            line = line.strip()
            if at == line:
                at_index = i
                break
        if at_index == -1:
            logging.warning("target not found: %s", at)
            return

        lines[at_index] = lines[at_index].replace(at, data)

    def regex_replace_if_exist(lines, at: str, data: str):
        logging.debug("replacing %s at %s", data, at)

        at_index = -1
        for i, line in enumerate(lines):
            line = line.strip()
            if regex.match(at, line):
                at_index = i
                break
        if at_index == -1:
            logging.warning("target not found: %s", at)
            return

        lines[at_index] = data if data.endswith('\n') else data + '\n'

    op_map = {
        "insert_if_not_exist": op_insert_if_not_exist,
        "replace_if_exist": op_replace_if_exist,
        "regex_replace_if_exist": regex_replace_if_exist,
    } 

    for modifier in modifiers:
        file = modifier.get("file")
        if not file:
            raise Exception("file is required for modifier")
        fp = os.path.join(src_dir, file)
        if not os.path.exists(fp):
            logging.warning("file not found: %s", fp)
            continue
        ops = modifier.get("operations", [])
        if len(ops) == 0:
            continue
        logging.debug("applying %d operations to %s", len(ops), fp)
        with open(fp, "r", encoding="utf-8") as f:
            lines = f.readlines()
        for op in ops:
            opn = op.get("name")
            if opn not in op_map:
                logging.warning("unsupported operation: %s", op)
                continue
            op_map[opn](lines, op.get("at", ""), op.get("data", ""))
        with open(fp, "w", encoding="utf-8") as f:
            f.writelines(lines)


def fetch_components(
    config_path: str,
    output_dir: str,
):
    logging.info("loading config: %s", config_path)
    with open(config_path, "r", encoding="utf-8") as f:
        config_dict = json.load(f)

    logging.info("fetching components: %s", ", ".join(config_dict.keys()))
    temp_dir = os.path.join(output_dir, ".temp")
    os.makedirs(temp_dir, exist_ok=True)

    for name, config in config_dict.items():
        output_path = os.path.join(temp_dir, name)

        if config.get("git"):
            git_clone(config["git"], output_path)
        else:
            raise Exception("Only git is supported for now")

        synchronize_files(
            os.path.join(output_path, config.get("base_dir", "")),
            os.path.join(output_dir, name),
            config.get("includes", []),
            config.get("excludes", []),
        )

        apply_modifiers(
            os.path.join(output_dir, name),
            config.get("modifiers", []),
        )

    logging.info("cleaning up temp directory: %s", temp_dir)
    shutil.rmtree(temp_dir)


def parse_args():
    parser = argparse.ArgumentParser(description="Fetch components")
    parser.add_argument("config", type=str, help="Config file path")
    parser.add_argument(
        "--output", type=str, help="Output directory", default="./components"
    )
    parser.add_argument("--verbose", action="store_true", help="Verbose mode")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    try:
        fetch_components(
            config_path=args.config,
            output_dir=args.output,
        )
    except Exception as e:
        logging.error(e)
        sys.exit(1)
