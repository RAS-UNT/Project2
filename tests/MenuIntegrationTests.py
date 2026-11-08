# Purpose: Tests the actual Milestone 1 application with scripted menu input.
# Author: Rami Badr
import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    """Runs isolated menu cases without changing the instructor's data files."""
    app = Path(sys.argv[1]).resolve()
    root = Path(__file__).resolve().parents[1]
    data = root / "data"
    originals = {path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in data.glob("*.txt")}
    passed = 0

    with tempfile.TemporaryDirectory(prefix="menu-cases-", dir=app.parent) as temporary:
        work = Path(temporary)

        def run(name, commands, expected=(), rejected=(), ordered=(), directory=None,
                arguments=None, exit_code=0):
            """Checks exit status, output markers, and their required ordering."""
            nonlocal passed
            args = arguments if arguments is not None else ([] if directory is None else [str(directory)])
            result = subprocess.run([str(app), *args], input=commands, text=True,
                                    capture_output=True, cwd=root, timeout=10)
            output = result.stdout + result.stderr
            if result.returncode != exit_code:
                raise RuntimeError(f"{name}: exit {result.returncode}, expected {exit_code}\n{output}")
            markers = ("Goodbye.", *expected) if exit_code == 0 else expected
            for marker in markers:
                if marker not in output:
                    raise RuntimeError(f"{name}: missing {marker!r}\n{output}")
            for marker in rejected:
                if marker in output:
                    raise RuntimeError(f"{name}: unexpected {marker!r}\n{output}")
            offset = 0
            for marker in ordered:
                position = output.find(marker, offset)
                if position < 0:
                    raise RuntimeError(f"{name}: missing or out-of-order {marker!r}\n{output}")
                offset = position + len(marker)
            print(f"PASS: {name}")
            passed += 1
            return output

        summaries = (
            "buildings.txt: 7 accepted, 0 rejected",
            "connections.txt: 6 accepted, 0 rejected",
            "facilities.txt: accepted=10 rejected=0 capacity=17 entries=10 collisions=0",
            "departments.txt: 7 accepted, 0 rejected",
        )
        run("startup_order_and_menu", "0\n", expected=(
            *summaries, "1. View all facilities", "2. Search facility by ID", "3. View campus graph",
            "4. Display graph counts and degrees", "5. Find reachable buildings (BFS)",
            "6. Add department", "7. Search department", "8. Display department hierarchy",
            "9. Run preorder traversal", "0. Exit"), ordered=summaries)
        run("all_facilities", "1\n0\n", expected=("ID: F001", "ID: F010", "Total facilities: 10"))
        run("facility_found", "2\n f004 \n0\n", expected=(
            "ID: F004\nName: Computer Lab A\nBuilding: Science Building\nCategory: Computer Lab",))
        run("facility_missing", "2\nf999\n0\n", expected=("Facility F999 was not found.",))
        for value in ("", "   ", "A001", "F01", "F0001", "F0A1", "F001suffix"):
            run("facility_invalid_" + repr(value), f"2\n{value}\n2\nf004\n0\n",
                expected=("Invalid facility ID.", "ID: F004"))
        run("graph_adjacency_order", "3\n0\n", expected=(
            "Library: Student Union, Business Building", "Observatory: (no connections)"))
        run("graph_counts_and_degrees", "4\n0\n", expected=(
            "Total buildings: 7", "Total connections: 6", "Library: 2", "Student Union: 3", "Observatory: 0"))
        run("bfs_connected", "5\n library \n0\n", expected=("Reachable: 6 of 7 buildings",), ordered=(
            "Library (distance 0)", "Student Union (distance 1)", "Business Building (distance 1)",
            "Science Building (distance 2)", "Gym (distance 2)", "Engineering Building (distance 3)"))
        run("bfs_isolated", "5\nOBSERVATORY\n0\n", expected=("Observatory (distance 0)", "Reachable: 1 of 7 buildings"))
        run("bfs_unknown_recovery", "5\nMissing Building\n4\n0\n", expected=(
            "unknown building 'Missing Building'", "Total buildings: 7"))
        run("bfs_blank", "5\n   \n0\n", expected=("building name cannot be blank",))
        run("bfs_internal_spaces", "5\nStudent  Union\n0\n", expected=("unknown building 'Student  Union'",))

        run("department_root_search", "7\n university \n0\n", expected=(
            "Department: University\nParent: None\nChildren: 2\nDepth: 0",))
        run("department_leaf_search", "7\n COMPUTER science \n0\n", expected=(
            "Department: Computer Science\nParent: College of Engineering\nChildren: 0\nDepth: 2",))
        run("department_missing", "7\nHistory\n0\n", expected=('Department "History" not found.',))
        run("department_blank_search", "7\n   \n0\n", expected=("department name cannot be blank",))
        hierarchy = ("University\n    College of Engineering\n        Computer Science\n"
                     "        Electrical Engineering\n    College of Science\n        Mathematics\n        Physics\n")
        preorder = ("University\nCollege of Engineering\nComputer Science\nElectrical Engineering\n"
                    "College of Science\nMathematics\nPhysics\n")
        run("department_hierarchy", "8\n0\n", expected=(hierarchy,))
        run("department_plain_preorder", "9\n0\n", expected=(preorder,), rejected=("    College of Engineering\n",))
        run("department_add_child", "6\nRobotics Lab\ncomputer science\n7\nrobotics lab\n8\n0\n",
            expected=('Added "Robotics Lab" under "Computer Science"', "Depth: 3", "hierarchy (8 departments)"))
        run("department_duplicate", "6\nCOMPUTER SCIENCE\nUniversity\n8\n0\n",
            expected=("duplicate department name", "hierarchy (7 departments)"))
        run("department_unknown_parent", "6\nRobotics\nMissing Parent\n0\n",
            expected=('parent department not found "Missing Parent"',))
        run("department_blank_add", "6\n  \n0\n", expected=("department name cannot be blank",),
            rejected=("Enter parent department name:",))
        run("department_blank_parent", "6\nRobotics\n   \n0\n", expected=("parent name cannot be blank",))

        empty = root / "testdata" / "facility-empty"
        run("all_empty", "1\n3\n4\n8\n9\n0\n", directory=empty, expected=(
            "buildings.txt: 0 accepted, 0 rejected", "connections.txt: 0 accepted, 0 rejected",
            "facilities.txt: accepted=0 rejected=0 capacity=17 entries=0 collisions=0",
            "departments.txt: 0 accepted, 0 rejected", "Total facilities: 0", "No buildings are loaded.",
            "The department tree is empty."))
        run("empty_add_root", "6\nSchool of Music\n7\nschool of music\n9\n0\n", directory=empty,
            expected=('Added "School of Music" as the root department.', "Parent: None\nChildren: 0\nDepth: 0"),
            rejected=("Enter parent department name:",))
        run("empty_root_then_child", "6\nSchool of Music\n6\nJazz Studies\nschool of music\n7\nJazz Studies\n0\n",
            directory=empty, expected=('Added "Jazz Studies" under "School of Music"', "Depth: 1"))
        invalid = root / "testdata" / "integration-invalid"
        output = run("invalid_records_recover", "3\n4\n1\n2\nf014\n5\nLIBRARY\n8\n9\n0\n", directory=invalid,
            expected=("buildings.txt: 3 accepted, 2 rejected", "connections.txt: 1 accepted, 5 rejected",
                      "facilities.txt: accepted=2 rejected=5 capacity=17 entries=2 collisions=1",
                      "departments.txt: 3 accepted, 7 rejected", "Library: Student Union",
                      "Observatory: (no connections)", "Total facilities: 2", "Category: Classroom",
                      "Reachable: 2 of 3 buildings", "University\nCollege\nLate Parent\n"))
        if output.count("WARNING: ") + output.count("Warning: ") != 19:
            raise RuntimeError("invalid_records_recover: expected one warning for each of 19 rejected records")
        run("collision_summary_in_app", "1\n2\nf014\n0\n", directory=root / "testdata" / "hash-collision",
            expected=("facilities.txt: accepted=3 rejected=2 capacity=17 entries=3 collisions=2",
                      "Name: Second Collision Facility", "Total facilities: 3"))

        windows = work / "windows data"
        windows.mkdir()
        for path in data.glob("*.txt"):
            content = path.read_bytes().replace(b"\r\n", b"\n").replace(b"F004|", b"f004|")
            (windows / path.name).write_bytes(content.replace(b"\n", b"\r\n"))
        run("crlf_and_directory_spaces", "2\nf004\n5\nLibrary\n9\n0\n", directory=windows,
            expected=(*summaries, "ID: F004", "Building: Science Building", "Reachable: 6 of 7 buildings", preorder))
        missing = work / "missing-files"
        missing.mkdir()
        run("all_files_missing", "1\n4\n8\n0\n", directory=missing, expected=(
            "Could not open file buildings.txt", "Could not open file connections.txt", "cannot open facility file",
            "could not open", "departments.txt: 0 accepted, 0 rejected", "Total facilities: 0", "Total buildings: 0"))
        missing_departments = work / "missing-departments"
        missing_departments.mkdir()
        for path in data.glob("*.txt"):
            if path.name != "departments.txt":
                (missing_departments / path.name).write_bytes(path.read_bytes())
        run("missing_department_add_root", "6\nReplacement Root\n7\nReplacement Root\n0\n",
            directory=missing_departments, expected=("departments.txt: 0 accepted, 0 rejected",
            'Added "Replacement Root" as the root department.', "Parent: None\nChildren: 0\nDepth: 0"),
            rejected=("Enter parent department name:",))
        reserved = work / "department-named-none"
        reserved.mkdir()
        for name in ("buildings.txt", "connections.txt", "facilities.txt"):
            (reserved / name).write_text("", encoding="utf-8")
        (reserved / "departments.txt").write_text("NONE|NONE\nChild|NONE\n", encoding="utf-8")
        run("existing_parent_named_none", "7\nChild\n9\n0\n", directory=reserved,
            expected=("departments.txt: 2 accepted, 0 rejected", "Parent: NONE\nChildren: 0\nDepth: 1", "NONE\nChild\n"))

        for value in ("", " ", "abc", "-1", "10", "1abc", "1.5", "2147483648", "1 2"):
            run("invalid_menu_" + repr(value), value + "\n4\n0\n",
                expected=("Invalid menu choice.", "Total buildings: 7"))
        for name, commands in (("eof_at_menu", ""), ("eof_at_id", "2\n"), ("eof_at_bfs", "5\n"),
                               ("eof_at_department", "6\n"), ("eof_at_parent", "6\nRobotics\n")):
            run(name, commands)
        run("exit_without_final_newline", "0")
        run("blank_directory_argument", "", arguments=[" "], exit_code=1, expected=("cannot be blank",))
        run("extra_arguments", "", arguments=["first", "second"], exit_code=1, expected=("Usage:",))

    after = {path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in data.glob("*.txt")}
    if originals != after:
        raise RuntimeError("Instructor data changed during menu operations")
    print(f"All {passed} integrated menu cases passed; instructor data unchanged.")


if __name__ == "__main__":
    main()
