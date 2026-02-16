# Remote Sensor MQTT API

## Description

This repo contains the definition of the API used to interact with the remote sensor nodes.

It needs to be integrated in both the sensor-master side, and the sensor side as a git submodule.

You can add this API definition to your project by doing:

```
mkdir doc
cd doc
git submodule add git@gitlab.internal.ainguraiiot.com:sw/remote_sensing/remote-sensor-mqtt-api.git mqtt-api
```

## API Tools

A few tools can be used to aid in viewing the API in a more user-friendly way:

* [asyncapi.asyncapi-preview vscodium extension](https://open-vsx.org/vscode/item?itemName=asyncapi.asyncapi-preview)
* [asyncapi studio online](https://studio.asyncapi.com/)
* [json schema faker](https://json-schema-faker.js.org/)
  - Check option alwaysFakeOptionals
  - Unckeck option fillProperties
* [asyncapi cli](https://www.asyncapi.com/tools/cli)
  - [Installation](https://github.com/asyncapi/cli/blob/master/docs/installation.md)
  - [Documentation](https://github.com/asyncapi/cli/blob/master/docs/usage.md)
  - Useful commands:
    -     asyncapi bundle -d .\mqtt\ .\mqtt_protocol_AI.yaml --output asyncapi.yaml
    -     asyncapi generate fromTemplate asyncapi.yaml @asyncapi/html-template@3.0.0 --use-new-generator -o .\html\

## Quality checks

This repo includes a file named `verified_srcrevs.json` aimed at assisting developers in keeping source code and API documentation up to date.

Repos implementing parts of the API documented in this repo can optionally incorporate a git-hook that can check if source code and API documentation are up to date.

See `verified_srcrevs.schema.json` for a full description on what is needed in this repository for automated git pre-push QC.

A sample implementation of the git-hook can be found [in this repo](https://gitlab.internal.ainguraiiot.com/sw/remote_sensing/ade9000/-/blob/develop/scripts/git-hooks/pre-push)

The idea is that this repo should be setup as a git submodule of any other repository containing code that implements the described API, and developers need to maintain both repos in sync to be able to pass the QC.

### Sample QC workflows

#### Scenario A: Documentation changes before source code

- New requirement
- Documentation is generated in a dedicated branch
- srcrev is set to an invalid value in that dedicated branch. commit and push.
- source code is updated accordingly in a branch with the same name
- source code modifications are commited, but cannot be pushed at this point
- update the documentation to reflect the last code update commit. commit and push
- update documentation submodule in the source code repo, commit and push

#### Scenario B: Source code changes before documentation

In the event that the source code changes before the documentation does (which should not happen), there is also a workflow to have this under control.

- Source code changes. Cannot be pushed.
- Review and update documentation, and asociated SRCREV, commit and push.
- Update documentation submodule in source code repository. Commit and push.
