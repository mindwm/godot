def can_build(env, platform):
    if platform in ("linuxbsd"):
        return True
    else:
        # not supported on these platforms
        return False


def configure(env):
    pass
