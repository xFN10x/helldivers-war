# THIS FILE WAS MADE WITH CHATGPT!

#!/usr/bin/env python3
"""Build Pebble Draw Command Sequences (PDCS) from PDCI frame files.

Designed for PDC files produced by pdc_tool. It does not re-encode drawing
commands: it extracts the PDCI command list verbatim and wraps each one in a
PDCS animation frame (duration + command list).

Usage:
    python3 pdc_sequence_builder.py frames/ animation.pdc --duration 100
    python3 pdc_sequence_builder.py frames/ animation.pdc --durations 33,50,33
    python3 pdc_sequence_builder.py frames/ animation.pdc --play-count 1

Input filenames are sorted naturally (frame2.pdc before frame10.pdc).
A duration can be supplied globally with --duration, or per frame with
--durations / --durations-file. If none is supplied, the script looks for a
leading integer followed by optional separator and an integer ending in 'ms'
in each filename, e.g. 0001_33ms.pdc. Otherwise it exits with an error.

MADE BY CHATGPT... I CANNOT FOR THE LIVE OF ME FIND A WORKING TOOL FOR THIS!
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

PDCI_HEADER_SIZE = 14
PDCS_HEADER_SIZE = 18
MAGIC_PDCI = b"PDCI"
MAGIC_PDCS = b"PDCS"
VERSION = 1
MAX_U16 = 0xFFFF


class FormatError(ValueError):
    pass


@dataclass(frozen=True)
class PDCIFrame:
    path: Path
    payload: bytes
    width: int
    height: int
    version: int
    reserved: int


@dataclass(frozen=True)
class PDCSFrame:
    duration_ms: int
    command_list: bytes


def natural_key(path: Path):
    parts = re.split(r"(\d+)", path.name)
    return [int(p) if p.isdigit() else p.lower() for p in parts]


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def i16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<h", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def validate_command_list(data: bytes, *, source: str) -> int:
    """Validate a Draw Command List and return its exact byte length."""
    if len(data) < 2:
        raise FormatError(f"{source}: command list is truncated")

    count = u16(data, 0)
    if count == 0:
        raise FormatError(f"{source}: command count is zero (invalid)")

    pos = 2
    for cmd_index in range(count):
        if pos + 9 > len(data):
            raise FormatError(
                f"{source}: command {cmd_index} header is truncated at offset {pos}"
            )

        cmd_type, flags, stroke, stroke_width, fill = data[pos : pos + 5]
        aux = u16(data, pos + 5)
        point_count = u16(data, pos + 7)

        if cmd_type not in (1, 2, 3):
            raise FormatError(
                f"{source}: command {cmd_index} has invalid type {cmd_type}"
            )
        if flags & 0xFE:
            # Bit 0 is the documented hidden bit; the other bits are reserved.
            raise FormatError(
                f"{source}: command {cmd_index} uses reserved flag bits: 0x{flags:02x}"
            )
        if point_count == 0:
            raise FormatError(f"{source}: command {cmd_index} has zero points")
        if cmd_type == 2 and point_count != 1:
            raise FormatError(
                f"{source}: circle command {cmd_index} must contain exactly one point"
            )
        if cmd_type == 2:
            # For circles, bytes 5..6 are radius rather than path-open flags.
            _ = aux
        _ = stroke, stroke_width, fill

        size = 9 + point_count * 4
        end = pos + size
        if end > len(data):
            raise FormatError(
                f"{source}: command {cmd_index} overruns command list "
                f"(needs {size} bytes at offset {pos}, only {len(data) - pos} remain)"
            )
        pos = end

    if pos != len(data):
        raise FormatError(
            f"{source}: {len(data) - pos} trailing byte(s) after command list"
        )

    return pos


def read_pdci(path: Path) -> PDCIFrame:
    data = path.read_bytes()
    if len(data) < PDCI_HEADER_SIZE:
        raise FormatError(f"{path}: only {len(data)} bytes; PDCI header is 14 bytes")
    if data[:4] != MAGIC_PDCI:
        raise FormatError(f"{path}: expected PDCI magic, found {data[:4]!r}")

    stored_size = u32(data, 4)
    actual_payload_size = len(data) - 8
    if stored_size != actual_payload_size:
        raise FormatError(
            f"{path}: PDCI payload size says {stored_size} bytes, "
            f"but file contains {actual_payload_size} bytes"
        )

    version = data[8]
    reserved = data[9]
    if version != VERSION:
        raise FormatError(f"{path}: unsupported PDCI version {version}; expected 1")
    if reserved != 0:
        raise FormatError(f"{path}: reserved byte is {reserved}; expected 0")

    width = i16(data, 10)
    height = i16(data, 12)
    if width <= 0 or height <= 0:
        raise FormatError(f"{path}: invalid view box {width}x{height}")

    command_list = data[PDCI_HEADER_SIZE:]
    validate_command_list(command_list, source=str(path))

    return PDCIFrame(
        path=path,
        payload=command_list,
        width=width,
        height=height,
        version=version,
        reserved=reserved,
    )


def parse_durations_csv(value: str) -> list[int]:
    try:
        values = [int(x.strip()) for x in value.split(",") if x.strip()]
    except ValueError as exc:
        raise argparse.ArgumentTypeError("durations must be comma-separated integers") from exc
    if not values:
        raise argparse.ArgumentTypeError("duration list is empty")
    return values


def validate_duration(value: str) -> int:
    try:
        value = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            "duration must be an integer number of milliseconds"
        ) from exc

    if not 0 <= value <= MAX_U16:
        raise argparse.ArgumentTypeError(
            "duration must be in the range 0..65535 ms"
        )

    return value


def parse_manifest_durations(path: Path) -> list[int]:
    values: list[int] = []
    for line_no, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        try:
            value = int(line)
        except ValueError as exc:
            raise FormatError(f"{path}:{line_no}: invalid duration {line!r}") from exc
        if not 0 <= value <= MAX_U16:
            raise FormatError(f"{path}:{line_no}: duration {value} is outside 0..65535")
        values.append(value)
    if not values:
        raise FormatError(f"{path}: no durations found")
    return values


def durations_from_filenames(paths: Sequence[Path]) -> list[int]:
    # Accept names such as 0001_33ms.pdc, frame-002-100ms.pdc, 3_250ms.pdc.
    pattern = re.compile(r"(?:^|[_-])(\d+)ms\.pdc$", re.IGNORECASE)
    values: list[int] = []
    for path in paths:
        match = pattern.search(path.name)
        if not match:
            raise FormatError(
                f"{path}: no duration found in filename; use --duration, "
                "--durations, or --durations-file"
            )
        value = int(match.group(1))
        if not 0 <= value <= MAX_U16:
            raise FormatError(f"{path}: duration {value} is outside 0..65535")
        values.append(value)
    return values


def build_pdcs(
    frames: Sequence[PDCIFrame],
    durations_ms: Sequence[int],
    *,
    play_count: int,
) -> bytes:
    if not frames:
        raise FormatError("no PDC frames supplied")
    if len(frames) != len(durations_ms):
        raise FormatError(
            f"received {len(frames)} frames but {len(durations_ms)} durations"
        )
    if not 0 <= play_count <= MAX_U16:
        raise FormatError("play count must be in the range 0..65535")
    if len(frames) > MAX_U16:
        raise FormatError("frame count exceeds 65535")

    width = frames[0].width
    height = frames[0].height
    for frame in frames[1:]:
        if frame.width != width or frame.height != height:
            raise FormatError(
                f"{frame.path}: view box is {frame.width}x{frame.height}, "
                f"but first frame is {width}x{height}"
            )

    body = bytearray()
    for frame, duration in zip(frames, durations_ms):
        if not 0 <= duration <= MAX_U16:
            raise FormatError(f"duration {duration} is outside 0..65535 ms")
        body += struct.pack("<H", duration)
        body += frame.payload

    # PDCS file = 8-byte outer header + 10-byte sequence header + frames.
    # The stored size is the number of bytes after the 8-byte magic/size pair.
    sequence_payload = struct.pack(
        "<BBhhHH",
        VERSION,
        0,
        width,
        height,
        play_count,
        len(frames),
    ) + body
    return MAGIC_PDCS + struct.pack("<I", len(sequence_payload)) + sequence_payload


def validate_pdcs(data: bytes) -> dict[str, int]:
    if len(data) < PDCS_HEADER_SIZE:
        raise FormatError("output is shorter than the 18-byte PDCS header")
    if data[:4] != MAGIC_PDCS:
        raise FormatError("output does not start with PDCS")

    stored_size = u32(data, 4)
    actual_payload_size = len(data) - 8
    if stored_size != actual_payload_size:
        raise FormatError(
            f"PDCS size field says {stored_size}, actual payload is {actual_payload_size}"
        )

    version = data[8]
    reserved = data[9]
    width = i16(data, 10)
    height = i16(data, 12)
    play_count = u16(data, 14)
    frame_count = u16(data, 16)

    if version != VERSION:
        raise FormatError(f"unsupported PDCS version {version}")
    if reserved != 0:
        raise FormatError(f"PDCS reserved byte is {reserved}, expected 0")
    if width <= 0 or height <= 0:
        raise FormatError(f"invalid PDCS view box {width}x{height}")
    if frame_count == 0:
        raise FormatError("PDCS frame count is zero")

    pos = PDCS_HEADER_SIZE
    for index in range(frame_count):
        if pos + 4 > len(data):
            raise FormatError(f"frame {index}: truncated duration/command-list header")
        duration = u16(data, pos)
        command_list = data[pos + 2 :]
        # Parse one command list directly from the current frame, but stop at
        # exactly one list. We need a small parser that returns consumed bytes.
        if len(command_list) < 2:
            raise FormatError(f"frame {index}: truncated command count")
        count = u16(command_list, 0)
        if count == 0:
            raise FormatError(f"frame {index}: command count is zero")
        q = 2
        for cmd_index in range(count):
            if q + 9 > len(command_list):
                raise FormatError(f"frame {index}, command {cmd_index}: truncated")
            cmd_type = command_list[q]
            flags = command_list[q + 1]
            point_count = u16(command_list, q + 7)
            if cmd_type not in (1, 2, 3):
                raise FormatError(f"frame {index}, command {cmd_index}: bad type {cmd_type}")
            if flags & 0xFE:
                raise FormatError(f"frame {index}, command {cmd_index}: reserved flags set")
            if point_count == 0:
                raise FormatError(f"frame {index}, command {cmd_index}: zero points")
            if cmd_type == 2 and point_count != 1:
                raise FormatError(f"frame {index}, circle command has {point_count} points")
            q += 9 + point_count * 4
            if q > len(command_list):
                raise FormatError(f"frame {index}, command {cmd_index}: overruns file")
        pos += 2 + q
        if pos > len(data):
            raise FormatError(f"frame {index} overruns file")
        _ = duration

    if pos != len(data):
        raise FormatError(f"{len(data) - pos} trailing bytes after final frame")

    return {
        "bytes": len(data),
        "payload_bytes": stored_size,
        "width": width,
        "height": height,
        "play_count": play_count,
        "frame_count": frame_count,
    }


def discover_frames(directory: Path) -> list[Path]:
    if not directory.is_dir():
        raise FormatError(f"not a directory: {directory}")
    paths = sorted((p for p in directory.iterdir() if p.is_file() and p.suffix.lower() == ".pdc"), key=natural_key)
    if not paths:
        raise FormatError(f"no .pdc files found in {directory}")
    return paths


def make_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Build a valid Pebble PDCS sequence from pdc_tool-generated PDCI frames."
    )
    parser.add_argument("frames_dir", type=Path, help="directory containing frame .pdc files")
    parser.add_argument("output", type=Path, help="output PDCS .pdc path")
    duration = parser.add_mutually_exclusive_group()
    duration.add_argument("--duration", type=validate_duration, help="same duration for every frame (ms)")
    duration.add_argument("--durations", type=parse_durations_csv, help="comma-separated per-frame durations (ms)")
    duration.add_argument("--durations-file", type=Path, help="text file containing one duration per frame (ms)")
    parser.add_argument(
        "--play-count",
        type=lambda x: int(x, 0),
        default=1,
        help="number of plays; 0 means no playback, 0xffff means forever (default: 1)",
    )
    parser.add_argument(
        "--filename-durations",
        action="store_true",
        help="read durations from filenames ending in '_NNNms.pdc' when no explicit duration option is used",
    )
    parser.add_argument("--dry-run", action="store_true", help="validate and report without writing output")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = make_arg_parser()
    args = parser.parse_args(argv)

    try:
        paths = discover_frames(args.frames_dir)
        frames = [read_pdci(path) for path in paths]

        if args.duration is not None:
            durations = [args.duration] * len(frames)
        elif args.durations is not None:
            durations = args.durations
        elif args.durations_file is not None:
            durations = parse_manifest_durations(args.durations_file)
        elif args.filename_durations:
            durations = durations_from_filenames(paths)
        else:
            parser.error(
                "provide --duration, --durations, --durations-file, "
                "or --filename-durations"
            )

        if len(durations) != len(frames):
            raise FormatError(
                f"duration count ({len(durations)}) does not match frame count ({len(frames)})"
            )

        result = build_pdcs(frames, durations, play_count=args.play_count)
        info = validate_pdcs(result)

        print(
            f"validated: {info['frame_count']} frames, "
            f"{info['width']}x{info['height']}, "
            f"{info['bytes']} bytes, play_count={info['play_count']}"
        )
        for i, (path, duration) in enumerate(zip(paths, durations)):
            print(f"  {i:03d}: {path.name} -> {duration} ms")

        if not args.dry_run:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(result)
            print(f"wrote: {args.output}")

        return 0
    except (OSError, FormatError, ValueError, struct.error) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
