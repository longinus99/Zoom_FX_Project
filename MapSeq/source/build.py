from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent.parent           # src/custom/mapseq/build.py -> repo root
sys.path.insert(0, str(ROOT / "build"))
sys.path.insert(0, str(ROOT / "src" / "airwindows" / "common"))

from linker import LinkerConfig, link, params_from_manifest  # noqa: E402
from manifest_params import write_param_header  # noqa: E402
from custom_covers import make_cover  # noqa: E402

def _find_ti_root() -> Path:
    """Locate the TI C6000 compiler folder (Windows or macOS)."""
    candidates = []
    if os.environ.get("TI_CGT_ROOT"):
        candidates.append(Path(os.environ["TI_CGT_ROOT"]))
    candidates += [
        Path.home() / "Downloads" / "ti-cgt-c6000_8.5.0.LTS",
        Path("C:/ti/ti-cgt-c6000_8.5.0.LTS"),
        Path("/Applications/ti/ti-cgt-c6000_8.5.0.LTS"),
    ]
    for c in candidates:
        if (c / "bin" / "cl6x.exe").exists() or (c / "bin" / "cl6x").exists():
            return c
    raise SystemExit(
        "Could not find the TI C6000 compiler (bin\\cl6x.exe).\n"
        "Set TI_CGT_ROOT to the folder that contains 'bin' and 'include', e.g.\n"
        "  set TI_CGT_ROOT=C:\\ti\\ti-cgt-c6000_8.5.0.LTS")


TI_ROOT = _find_ti_root()
CL6X = TI_ROOT / "bin" / ("cl6x.exe" if (TI_ROOT / "bin" / "cl6x.exe").exists() else "cl6x")

CFLAGS = [
    "--c99",
    "--opt_level=2",
    "-mv6740",
    "--abi=eabi",
    "--mem_model:data=far",
    f"--include_path={TI_ROOT.as_posix()}/include",
]


def main() -> None:
    manifest = json.loads((HERE / "manifest_pedal.json").read_text(encoding="utf-8"))
    write_param_header(manifest, HERE / "mapseq_params.h", "MAPSEQ")

    src_c = HERE / "mapseq.c"
    out_dir = ROOT / "dist"
    out_dir.mkdir(exist_ok=True)

    effect_name = manifest["effect_name"]
    if len(effect_name) > 8:
        raise ValueError(f"effect_name {effect_name!r} is longer than 8 characters")
    audio_func = manifest["audio_func_name"]
    obj = HERE / f"{effect_name.lower()}.obj"
    out_zdl = out_dir / f"{effect_name}.ZDL"

    print(f"[mapseq] compiling {src_c.name} -> {obj.name}")
    subprocess.run(
        [
            str(CL6X),
            *CFLAGS,
            "-c",
            str(src_c),
            f"--output_file={obj}",
        ],
        check=True,
        cwd=HERE,
    )

    for junk in ("compiler.opt", "linker.cmd"):
        p = HERE / junk
        if p.exists():
            p.unlink()

    cfg = LinkerConfig(
        materialize_init=True,
        effect_name=effect_name,
        screen_image=make_cover(effect_name, [p["name"] for p in manifest["params"]]),
        audio_func_name=audio_func,
        gid=manifest["gid"],
        fxid=manifest["fxid"],
        params=params_from_manifest(manifest["params"]),
        obj_path=obj,
        output_path=out_zdl,
        fxid_version=manifest.get("fxid_version", "1.00").encode("ascii"),
        flags_byte=manifest.get("flags_byte", 0x01),
        dsp_cost=manifest.get("dsp_cost", 20.0),
        audio_nop=manifest.get("audio_nop", False),
        use_object_edit_handlers=False,
        synthesize_linesel_edit_handlers=True,
        synth_edit_start_index=2,
        knob3_blob_path="/tmp/__nonexistent__",
    )
    link(cfg)

    print(f"\n[mapseq] done -> {out_zdl}")


if __name__ == "__main__":
    main()
