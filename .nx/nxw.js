// This file should be committed to your repository! It wraps Nx and ensures
// that your local installation matches nx.json.
// See: https://nx.dev/recipes/installation/install-non-javascript for more info.

import { existsSync, mkdirSync, writeFileSync, readFileSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';
import { execSync } from 'child_process';
import { createRequire } from 'module';

const __dirname = dirname(fileURLToPath(import.meta.url));
const require = createRequire(import.meta.url);
const installationPath = join(__dirname, 'installation', 'package.json');

function matchesCurrentNxInstall(currentInstallation, nxJsonInstallation) {
    if (!currentInstallation.devDependencies ||
        !Object.keys(currentInstallation.devDependencies).length) {
        return false;
    }
    try {
        if (currentInstallation.devDependencies['nx'] !== nxJsonInstallation.version ||
            JSON.parse(readFileSync(join(dirname(installationPath), 'node_modules', 'nx', 'package.json'), 'utf-8')).version !== nxJsonInstallation.version) {
            return false;
        }
        for (const [plugin, desiredVersion] of Object.entries(nxJsonInstallation.plugins || {})) {
            if (currentInstallation.devDependencies[plugin] !== desiredVersion) {
                return false;
            }
        }
        return true;
    } catch {
        return false;
    }
}

function ensureDir(p) {
    if (!existsSync(p)) {
        mkdirSync(p, { recursive: true });
    }
}

function getCurrentInstallation() {
    try {
        return JSON.parse(readFileSync(installationPath, 'utf-8'));
    } catch {
        return { name: 'nx-installation', version: '0.0.0', devDependencies: {} };
    }
}

function performInstallation(currentInstallation, nxJson) {
    writeFileSync(installationPath, JSON.stringify({
        name: 'nx-installation',
        devDependencies: {
            nx: nxJson.installation.version,
            ...nxJson.installation.plugins,
        },
    }));
    try {
        execSync('npm i', {
            cwd: dirname(installationPath),
            stdio: 'inherit',
            windowsHide: false,
        });
    } catch (e) {
        writeFileSync(installationPath, JSON.stringify(currentInstallation));
        throw e;
    }
}

function ensureUpToDateInstallation() {
    const nxJsonPath = join(__dirname, '..', 'nx.json');
    let nxJson;
    try {
        nxJson = JSON.parse(readFileSync(nxJsonPath, 'utf-8'));
        if (!nxJson.installation) {
            console.error('[NX]: The "installation" entry in the "nx.json" file is required when running the nx wrapper. See https://nx.dev/recipes/installation/install-non-javascript');
            process.exit(1);
        }
    } catch {
        console.error('[NX]: The "nx.json" file is required when running the nx wrapper. See https://nx.dev/recipes/installation/install-non-javascript');
        process.exit(1);
    }
    try {
        ensureDir(join(__dirname, 'installation'));
        const currentInstallation = getCurrentInstallation();
        if (!matchesCurrentNxInstall(currentInstallation, nxJson.installation)) {
            performInstallation(currentInstallation, nxJson);
        }
    } catch (e) {
        const messageLines = ['[NX]: Nx wrapper failed to synchronize installation.'];
        if (e instanceof Error) {
            messageLines.push('', e.message, e.stack);
        } else {
            messageLines.push(e.toString());
        }
        console.error(messageLines.join('\n'));
        process.exit(1);
    }
}

if (!process.env.NX_WRAPPER_SKIP_INSTALL) {
    ensureUpToDateInstallation();
}

require('./installation/node_modules/nx/bin/nx');
