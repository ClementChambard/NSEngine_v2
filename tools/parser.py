import argparse
from commands import engine as cmd_engine


def _engine_parser(subparsers):
    parser_engine = subparsers.add_parser("engine", help="engine related commands")

    engine_subparsers = parser_engine.add_subparsers(help="subcommands")
    engine_subparsers.required = True

    configure_parser = engine_subparsers.add_parser("configure")
    configure_parser.set_defaults(func=cmd_engine.configure)

    git_parser = engine_subparsers.add_parser("git")
    git_parser.set_defaults(func=cmd_engine.git)

    build_parser = engine_subparsers.add_parser("build")
    build_parser.set_defaults(func=cmd_engine.build)

    edit_parser = engine_subparsers.add_parser("edit")
    edit_parser.set_defaults(func=cmd_engine.edit)

    run_parser = engine_subparsers.add_parser("run")
    run_parser.set_defaults(func=cmd_engine.run)

    debug_parser = engine_subparsers.add_parser("debug")
    debug_parser.set_defaults(func=cmd_engine.debug)

    test_parser = engine_subparsers.add_parser("test")
    test_parser.set_defaults(func=cmd_engine.test)

    clean_parser = engine_subparsers.add_parser("clean")
    clean_parser.set_defaults(func=cmd_engine.clean)
    clean_parser.add_argument(
        "-a", "--all", action="store_true", help="clean everything"
    )


def create():
    parser = argparse.ArgumentParser(prog="ns")

    subparsers = parser.add_subparsers(
        dest="command", title="subcommands", description="valid subcommands"
    )
    subparsers.required = True

    _engine_parser(subparsers)
    return parser
